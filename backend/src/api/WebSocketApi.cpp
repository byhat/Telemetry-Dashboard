#include "telemetry/api/WebSocketApi.h"
#include "telemetry/models/TelemetryData.h"
#include "telemetry/models/ErrorMessage.h"
#include <sstream>

namespace telemetry {
namespace api {

WebSocketApi::WebSocketApi(std::shared_ptr<services::WebSocketService> ws_service,
                           std::shared_ptr<services::TelemetryService> telemetry_service,
                           std::shared_ptr<services::ErrorService> error_service)
    : m_wsService(std::move(ws_service))
    , m_telemetryService(std::move(telemetry_service))
    , m_errorService(std::move(error_service))
{
}

void WebSocketApi::onOpen(crow::websocket::connection& conn) {
    std::string connection_id = m_wsService->addConnection(&conn);
    m_wsService->handleConnection(connection_id);
    
    // Send welcome message
    crow::json::wvalue welcome;
    welcome["type"] = "status";
    welcome["message"] = "Connected to telemetry server";
    welcome["connection_id"] = connection_id;
    
    conn.send_text(welcome.dump());
}

void WebSocketApi::onMessage(crow::websocket::connection& conn, const std::string& message, bool is_binary) {
    if (is_binary) {
        // Binary messages not supported
        return;
    }
    
    std::string connection_id = m_wsService->getConnectionId(&conn);
    if (connection_id.empty()) {
        return;
    }
    
    try {
        auto json = crow::json::load(message);
        if (!json) {
            sendError(connection_id, "Invalid JSON format");
            return;
        }
        
        std::string type = json["type"].s();
        
        if (type == "telemetry") {
            handleTelemetryMessage(connection_id, message);
        } else if (type == "error") {
            handleErrorMessage(connection_id, message);
        } else if (type == "subscribe") {
            handleSubscribeMessage(connection_id, message);
        } else if (type == "unsubscribe") {
            handleUnsubscribeMessage(connection_id, message);
        } else {
            sendError(connection_id, "Unknown message type: " + type);
        }
    } catch (const std::exception& e) {
        sendError(connection_id, std::string("Error processing message: ") + e.what());
    }
}

void WebSocketApi::onClose(crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
    std::string connection_id = m_wsService->getConnectionId(&conn);
    m_wsService->handleDisconnection(connection_id);
}

void WebSocketApi::handleTelemetryMessage(const std::string& connection_id, const std::string& message) {
    try {
        auto json = crow::json::load(message);
        std::string message_id = json.has("message_id") ? std::string(json["message_id"].s()) : "";
        
        models::TelemetryData data = models::jsonToTelemetryData(json);
        
        if (m_telemetryService->processTelemetry(data)) {
            sendAck(connection_id, "success", message_id);
        } else {
            sendError(connection_id, "Invalid telemetry data", message_id);
        }
    } catch (const std::exception& e) {
        sendError(connection_id, std::string("Error processing telemetry: ") + e.what());
    }
}

void WebSocketApi::handleErrorMessage(const std::string& connection_id, const std::string& message) {
    try {
        auto json = crow::json::load(message);
        std::string message_id = json.has("message_id") ? std::string(json["message_id"].s()) : "";
        
        models::ErrorMessage error = models::jsonToErrorMessage(json);
        
        if (m_errorService->processError(error)) {
            sendAck(connection_id, "success", message_id);
        } else {
            sendError(connection_id, "Invalid error data", message_id);
        }
    } catch (const std::exception& e) {
        sendError(connection_id, std::string("Error processing error: ") + e.what());
    }
}

void WebSocketApi::handleSubscribeMessage(const std::string& connection_id, const std::string& message) {
    try {
        auto json = crow::json::load(message);
        std::string device_id = json["device_id"].s();
        
        // For now, we broadcast to all clients, so subscription is implicit
        crow::json::wvalue response;
        response["type"] = "subscribed";
        response["device_id"] = device_id;
        
        m_wsService->sendToClient(connection_id, response.dump());
    } catch (const std::exception& e) {
        sendError(connection_id, std::string("Error processing subscribe: ") + e.what());
    }
}

void WebSocketApi::handleUnsubscribeMessage(const std::string& connection_id, const std::string& message) {
    try {
        auto json = crow::json::load(message);
        std::string device_id = json["device_id"].s();
        
        crow::json::wvalue response;
        response["type"] = "unsubscribed";
        response["device_id"] = device_id;
        
        m_wsService->sendToClient(connection_id, response.dump());
    } catch (const std::exception& e) {
        sendError(connection_id, std::string("Error processing unsubscribe: ") + e.what());
    }
}

void WebSocketApi::sendAck(const std::string& connection_id, const std::string& status, 
                           const std::string& message_id) {
    crow::json::wvalue response;
    response["type"] = "ack";
    response["status"] = status;
    
    if (!message_id.empty()) {
        response["message_id"] = message_id;
    }
    
    m_wsService->sendToClient(connection_id, response.dump());
}

void WebSocketApi::sendError(const std::string& connection_id, const std::string& error_text,
                             const std::string& message_id) {
    crow::json::wvalue response;
    response["type"] = "error";
    response["error"] = error_text;
    
    if (!message_id.empty()) {
        response["message_id"] = message_id;
    }
    
    m_wsService->sendToClient(connection_id, response.dump());
}

void WebSocketApi::broadcastTelemetryUpdate(const models::TelemetryData& data) {
    crow::json::wvalue update;
    update["type"] = "telemetry_update";
    update["device_id"] = data.device_id;
    update["temperature"] = data.temperature;
    update["power"] = data.power;
    update["voltage"] = data.voltage;
    update["timestamp"] = data.getTimestampMs();
    
    m_wsService->broadcast(update.dump());
}

void WebSocketApi::broadcastErrorUpdate(const models::ErrorMessage& error) {
    crow::json::wvalue update;
    update["type"] = "error_update";
    update["device_id"] = error.device_id;
    update["error_text"] = error.error_text;
    update["timestamp"] = error.getTimestampMs();
    
    m_wsService->broadcast(update.dump());
}

void WebSocketApi::registerRoutes(crow::SimpleApp& app) {
    // Register callbacks for telemetry and error broadcasts
    m_telemetryService->registerTelemetryCallback(
        [this](const models::TelemetryData& data) {
            broadcastTelemetryUpdate(data);
        }
    );
    
    m_errorService->registerErrorCallback(
        [this](const models::ErrorMessage& error) {
            broadcastErrorUpdate(error);
        }
    );
    
    // Register WebSocket endpoint
    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([this](crow::websocket::connection& conn) {
            onOpen(conn);
        })
        .onmessage([this](crow::websocket::connection& conn, const std::string& message, bool is_binary) {
            onMessage(conn, message, is_binary);
        })
        .onclose([this](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
            onClose(conn, reason, code);
        });
}

} // namespace api
} // namespace telemetry
