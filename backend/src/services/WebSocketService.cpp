#include "telemetry/services/WebSocketService.h"
#include <sstream>
#include <iomanip>
#include <chrono>

namespace telemetry {
namespace services {

WebSocketService::WebSocketService()
    : m_nextConnectionId(1)
{
}

WebSocketService::~WebSocketService() {
    clearAllConnections();
}

std::string WebSocketService::generateConnectionId() {
    std::ostringstream oss;
    oss << "conn_" << std::setw(6) << std::setfill('0') << m_nextConnectionId++;
    return oss.str();
}

std::string WebSocketService::addConnection(crow::websocket::connection* conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!conn) {
        return "";
    }
    
    std::string connection_id = generateConnectionId();
    
    m_connections[connection_id] = conn;
    m_connectionIds[conn] = connection_id;
    m_connectionIdSet.insert(connection_id);
    
    return connection_id;
}

void WebSocketService::removeConnection(crow::websocket::connection* conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!conn) {
        return;
    }
    
    auto it = m_connectionIds.find(conn);
    if (it != m_connectionIds.end()) {
        std::string connection_id = it->second;
        m_connections.erase(connection_id);
        m_connectionIdSet.erase(connection_id);
        m_connectionIds.erase(it);
    }
}

void WebSocketService::removeConnectionById(const std::string& connection_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connection_id);
    if (it != m_connections.end()) {
        crow::websocket::connection* conn = it->second;
        m_connectionIds.erase(conn);
        m_connections.erase(it);
        m_connectionIdSet.erase(connection_id);
    }
}

size_t WebSocketService::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t sent_count = 0;
    std::vector<std::string> failed_connections;
    
    for (const auto& pair : m_connections) {
        try {
            if (pair.second) {
                pair.second->send_text(message);
                sent_count++;
            }
        } catch (const std::exception& e) {
            // Connection may be closed, mark for removal
            failed_connections.push_back(pair.first);
        }
    }
    
    // Remove failed connections
    for (const auto& conn_id : failed_connections) {
        auto it = m_connections.find(conn_id);
        if (it != m_connections.end()) {
            m_connectionIds.erase(it->second);
            m_connections.erase(it);
            m_connectionIdSet.erase(conn_id);
        }
    }
    
    return sent_count;
}

bool WebSocketService::sendToClient(const std::string& connection_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connection_id);
    if (it == m_connections.end() || !it->second) {
        return false;
    }
    
    try {
        it->second->send_text(message);
        return true;
    } catch (const std::exception& e) {
        // Connection may be closed, remove it
        m_connectionIds.erase(it->second);
        m_connections.erase(it);
        m_connectionIdSet.erase(connection_id);
        return false;
    }
}

size_t WebSocketService::getConnectedClientCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.size();
}

bool WebSocketService::hasConnection(const std::string& connection_id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.find(connection_id) != m_connections.end();
}

void WebSocketService::clearAllConnections() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.clear();
    m_connectionIds.clear();
    m_connectionIdSet.clear();
}

crow::websocket::connection* WebSocketService::getConnection(const std::string& connection_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(connection_id);
    if (it != m_connections.end()) {
        return it->second;
    }
    return nullptr;
}

std::string WebSocketService::getConnectionId(crow::websocket::connection* conn) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connectionIds.find(conn);
    if (it != m_connectionIds.end()) {
        return it->second;
    }
    return "";
}

void WebSocketService::registerMessageHandler(MessageHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messageHandler = std::move(handler);
}

void WebSocketService::registerConnectionHandler(ConnectionHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connectionHandler = std::move(handler);
}

void WebSocketService::registerDisconnectionHandler(ConnectionHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_disconnectionHandler = std::move(handler);
}

void WebSocketService::handleMessage(const std::string& connection_id, const std::string& message) {
    MessageHandler handler;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        handler = m_messageHandler;
    }
    
    if (handler) {
        handler(connection_id, message);
    }
}

void WebSocketService::handleConnection(const std::string& connection_id) {
    ConnectionHandler handler;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        handler = m_connectionHandler;
    }
    
    if (handler) {
        handler(connection_id);
    }
}

void WebSocketService::handleDisconnection(const std::string& connection_id) {
    ConnectionHandler handler;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        handler = m_disconnectionHandler;
    }
    
    if (handler) {
        handler(connection_id);
    }
    
    // Always remove the connection
    removeConnectionById(connection_id);
}

} // namespace services
} // namespace telemetry
