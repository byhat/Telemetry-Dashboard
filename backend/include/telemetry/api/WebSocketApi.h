#pragma once

#include "telemetry/services/WebSocketService.h"
#include "telemetry/services/TelemetryService.h"
#include "telemetry/services/ErrorService.h"
#include <crow.h>
#include <memory>
#include <string>

namespace telemetry {
namespace api {

/**
 * @brief WebSocket API handler for real-time telemetry updates
 * 
 * This class provides WebSocket endpoints for real-time telemetry data
 * transmission, including receiving data from devices and broadcasting
 * updates to connected frontend clients.
 */
class WebSocketApi {
public:
    /**
     * @brief Constructor
     * @param ws_service Pointer to the WebSocket service
     * @param telemetry_service Pointer to the telemetry service
     * @param error_service Pointer to the error service
     */
    WebSocketApi(std::shared_ptr<services::WebSocketService> ws_service,
                 std::shared_ptr<services::TelemetryService> telemetry_service,
                 std::shared_ptr<services::ErrorService> error_service);
    
    /**
     * @brief Destructor
     */
    ~WebSocketApi() = default;
    
    /**
     * @brief Register all WebSocket routes with the Crow application
     * @param app Reference to the Crow application
     */
    void registerRoutes(crow::SimpleApp& app);
    
private:
    std::shared_ptr<services::WebSocketService> m_wsService;
    std::shared_ptr<services::TelemetryService> m_telemetryService;
    std::shared_ptr<services::ErrorService> m_errorService;
    
    /**
     * @brief Handle WebSocket connection open
     * @param conn Pointer to the WebSocket connection
     */
    void onOpen(crow::websocket::connection& conn);
    
    /**
     * @brief Handle WebSocket message received
     * @param conn Pointer to the WebSocket connection
     * @param message The received message
     * @param is_binary Whether the message is binary
     */
    void onMessage(crow::websocket::connection& conn, const std::string& message, bool is_binary);
    
    /**
     * @brief Handle WebSocket connection close
     * @param conn Pointer to the WebSocket connection
     * @param reason The reason for closing
     * @param code The close code
     */
    void onClose(crow::websocket::connection& conn, const std::string& reason, uint16_t code);
    
    /**
     * @brief Handle incoming telemetry message
     * @param connection_id The connection ID
     * @param message The message content
     */
    void handleTelemetryMessage(const std::string& connection_id, const std::string& message);
    
    /**
     * @brief Handle incoming error message
     * @param connection_id The connection ID
     * @param message The message content
     */
    void handleErrorMessage(const std::string& connection_id, const std::string& message);
    
    /**
     * @brief Handle subscribe message
     * @param connection_id The connection ID
     * @param message The message content
     */
    void handleSubscribeMessage(const std::string& connection_id, const std::string& message);
    
    /**
     * @brief Handle unsubscribe message
     * @param connection_id The connection ID
     * @param message The message content
     */
    void handleUnsubscribeMessage(const std::string& connection_id, const std::string& message);
    
    /**
     * @brief Send an acknowledgment message
     * @param connection_id The connection ID
     * @param status The status of the operation
     * @param message_id Optional message ID
     */
    void sendAck(const std::string& connection_id, const std::string& status, 
                 const std::string& message_id = "");
    
    /**
     * @brief Send an error message
     * @param connection_id The connection ID
     * @param error_text The error text
     * @param message_id Optional message ID
     */
    void sendError(const std::string& connection_id, const std::string& error_text,
                   const std::string& message_id = "");
    
    /**
     * @brief Broadcast telemetry update to all connected clients
     * @param data The telemetry data to broadcast
     */
    void broadcastTelemetryUpdate(const models::TelemetryData& data);
    
    /**
     * @brief Broadcast error update to all connected clients
     * @param error The error message to broadcast
     */
    void broadcastErrorUpdate(const models::ErrorMessage& error);
};

} // namespace api
} // namespace telemetry
