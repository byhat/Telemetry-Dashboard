#pragma once

#include <crow/websocket.h>
#include <string>
#include <unordered_set>
#include <mutex>
#include <functional>
#include <memory>

namespace telemetry {
namespace services {

/**
 * @brief Service for managing WebSocket connections and broadcasting messages
 * 
 * This class provides thread-safe management of WebSocket connections and
 * supports broadcasting messages to all connected clients. It handles
 * connection lifecycle events and maintains a list of active connections.
 */
class WebSocketService {
public:
    /**
     * @brief Constructor
     */
    WebSocketService();
    
    /**
     * @brief Destructor
     */
    ~WebSocketService();
    
    // Disable copy and move
    WebSocketService(const WebSocketService&) = delete;
    WebSocketService& operator=(const WebSocketService&) = delete;
    WebSocketService(WebSocketService&&) = delete;
    WebSocketService& operator=(WebSocketService&&) = delete;
    
    /**
     * @brief Add a new connection to the service
     * @param conn Pointer to the WebSocket connection
     * @return Unique connection ID for this connection
     */
    std::string addConnection(crow::websocket::connection* conn);
    
    /**
     * @brief Remove a connection from the service
     * @param conn Pointer to the WebSocket connection to remove
     */
    void removeConnection(crow::websocket::connection* conn);
    
    /**
     * @brief Remove a connection by its ID
     * @param connection_id The unique connection ID
     */
    void removeConnectionById(const std::string& connection_id);
    
    /**
     * @brief Broadcast a message to all connected clients
     * @param message The message to broadcast (JSON string)
     * @return Number of clients the message was sent to
     */
    size_t broadcast(const std::string& message);
    
    /**
     * @brief Send a message to a specific client
     * @param connection_id The unique connection ID
     * @param message The message to send (JSON string)
     * @return true if the message was sent successfully, false otherwise
     */
    bool sendToClient(const std::string& connection_id, const std::string& message);
    
    /**
     * @brief Get the number of currently connected clients
     * @return Number of connected clients
     */
    size_t getConnectedClientCount() const;
    
    /**
     * @brief Check if a specific connection is still active
     * @param connection_id The unique connection ID
     * @return true if the connection is active, false otherwise
     */
    bool hasConnection(const std::string& connection_id) const;
    
    /**
     * @brief Remove all connections
     */
    void clearAllConnections();
    
    /**
     * @brief Get a pointer to a connection by its ID
     * @param connection_id The unique connection ID
     * @return Pointer to the connection, or nullptr if not found
     */
    crow::websocket::connection* getConnection(const std::string& connection_id);
    
    /**
     * @brief Get the connection ID for a given connection pointer
     * @param conn Pointer to the WebSocket connection
     * @return The connection ID, or empty string if not found
     */
    std::string getConnectionId(crow::websocket::connection* conn) const;
    
    /**
     * @brief Message handler callback type
     */
    using MessageHandler = std::function<void(const std::string& connection_id, const std::string& message)>;
    
    /**
     * @brief Connection handler callback type
     */
    using ConnectionHandler = std::function<void(const std::string& connection_id)>;
    
    /**
     * @brief Register a callback for incoming messages
     * @param handler The callback function
     */
    void registerMessageHandler(MessageHandler handler);
    
    /**
     * @brief Register a callback for new connections
     * @param handler The callback function
     */
    void registerConnectionHandler(ConnectionHandler handler);
    
    /**
     * @brief Register a callback for disconnections
     * @param handler The callback function
     */
    void registerDisconnectionHandler(ConnectionHandler handler);
    
    /**
     * @brief Handle an incoming message from a connection
     * @param connection_id The unique connection ID
     * @param message The incoming message
     */
    void handleMessage(const std::string& connection_id, const std::string& message);
    
    /**
     * @brief Handle a new connection event
     * @param connection_id The unique connection ID
     */
    void handleConnection(const std::string& connection_id);
    
    /**
     * @brief Handle a disconnection event
     * @param connection_id The unique connection ID
     */
    void handleDisconnection(const std::string& connection_id);
    
private:
    /**
     * @brief Generate a unique connection ID
     * @return A unique connection ID string
     */
    std::string generateConnectionId();
    
    struct ConnectionInfo {
        crow::websocket::connection* connection;
        std::string id;
    };
    
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, crow::websocket::connection*> m_connections;
    std::unordered_map<crow::websocket::connection*, std::string> m_connectionIds;
    std::unordered_set<std::string> m_connectionIdSet;
    size_t m_nextConnectionId;
    
    MessageHandler m_messageHandler;
    ConnectionHandler m_connectionHandler;
    ConnectionHandler m_disconnectionHandler;
};

} // namespace services
} // namespace telemetry
