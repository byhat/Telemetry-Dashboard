#pragma once

#include "telemetry/storage/InMemoryStorage.h"
#include "telemetry/services/WebSocketService.h"
#include "telemetry/services/TelemetryService.h"
#include "telemetry/services/ErrorService.h"
#include "telemetry/api/RestApi.h"
#include "telemetry/api/WebSocketApi.h"
#include <crow.h>
#include <memory>
#include <atomic>
#include <thread>

namespace telemetry {

/**
 * @brief Main telemetry server class
 * 
 * This class orchestrates all components of the telemetry system including
 * storage, services, and API endpoints. It manages the Crow web application
 * and provides a clean interface for starting and stopping the server.
 */
class TelemetryServer {
public:
    /**
     * @brief Constructor
     * @param port The port number to listen on (default: 18080)
     * @param max_telemetry_points Maximum telemetry points per device (default: 1000)
     * @param max_error_points Maximum error points per device (default: 100)
     */
    explicit TelemetryServer(int port = 18080,
                            size_t max_telemetry_points = 1000,
                            size_t max_error_points = 100);
    
    /**
     * @brief Destructor
     */
    ~TelemetryServer();
    
    // Disable copy and move
    TelemetryServer(const TelemetryServer&) = delete;
    TelemetryServer& operator=(const TelemetryServer&) = delete;
    TelemetryServer(TelemetryServer&&) = delete;
    TelemetryServer& operator=(TelemetryServer&&) = delete;
    
    /**
     * @brief Start the server
     * @param bind_address The address to bind to (default: 0.0.0.0)
     */
    void start(const std::string& bind_address = "0.0.0.0");
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if the server is running
     * @return true if the server is running, false otherwise
     */
    bool isRunning() const;
    
    /**
     * @brief Get the port number
     * @return The port number the server is listening on
     */
    int getPort() const { return m_port; }
    
    /**
     * @brief Get the number of connected WebSocket clients
     * @return Number of connected clients
     */
    size_t getConnectedClientCount() const;
    
    /**
     * @brief Get access to the storage (for testing purposes)
     * @return Pointer to the storage instance
     */
    std::shared_ptr<storage::InMemoryStorage> getStorage() const { return m_storage; }
    
    /**
     * @brief Get access to the telemetry service (for testing purposes)
     * @return Pointer to the telemetry service instance
     */
    std::shared_ptr<services::TelemetryService> getTelemetryService() const { return m_telemetryService; }
    
    /**
     * @brief Get access to the error service (for testing purposes)
     * @return Pointer to the error service instance
     */
    std::shared_ptr<services::ErrorService> getErrorService() const { return m_errorService; }
    
    /**
     * @brief Get access to the WebSocket service (for testing purposes)
     * @return Pointer to the WebSocket service instance
     */
    std::shared_ptr<services::WebSocketService> getWebSocketService() const { return m_wsService; }
    
private:
    /**
     * @brief Initialize all components
     */
    void initialize();
    
    /**
     * @brief Register all routes with the Crow application
     */
    void registerRoutes();
    
    int m_port;
    size_t m_maxTelemetryPoints;
    size_t m_maxErrorPoints;
    
    // Components
    std::shared_ptr<storage::InMemoryStorage> m_storage;
    std::shared_ptr<services::WebSocketService> m_wsService;
    std::shared_ptr<services::TelemetryService> m_telemetryService;
    std::shared_ptr<services::ErrorService> m_errorService;
    std::unique_ptr<api::RestApi> m_restApi;
    std::unique_ptr<api::WebSocketApi> m_webSocketApi;
    
    // Crow application
    std::unique_ptr<crow::SimpleApp> m_app;
    
    // Server state
    std::atomic<bool> m_running;
    std::thread m_serverThread;
};

} // namespace telemetry
