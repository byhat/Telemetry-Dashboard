#include "telemetry/TelemetryServer.h"
#include <iostream>
#include <csignal>

namespace telemetry {

// Global pointer for signal handling
static TelemetryServer* g_server = nullptr;

void signalHandler(int signal) {
    if (g_server && g_server->isRunning()) {
        std::cout << "\nReceived signal " << signal << ", shutting down server..." << std::endl;
        g_server->stop();
    }
}

TelemetryServer::TelemetryServer(int port, size_t max_telemetry_points, size_t max_error_points)
    : m_port(port)
    , m_maxTelemetryPoints(max_telemetry_points)
    , m_maxErrorPoints(max_error_points)
    , m_running(false)
{
    initialize();
}

TelemetryServer::~TelemetryServer() {
    stop();
}

void TelemetryServer::initialize() {
    // Create storage
    m_storage = std::make_shared<storage::InMemoryStorage>(m_maxTelemetryPoints, m_maxErrorPoints);
    
    // Create services
    m_wsService = std::make_shared<services::WebSocketService>();
    m_telemetryService = std::make_shared<services::TelemetryService>(m_storage);
    m_errorService = std::make_shared<services::ErrorService>(m_storage);
    
    // Create API handlers
    m_restApi = std::make_unique<api::RestApi>(m_telemetryService, m_errorService);
    m_webSocketApi = std::make_unique<api::WebSocketApi>(m_wsService, m_telemetryService, m_errorService);
    
    // Create Crow application
    m_app = std::make_unique<crow::SimpleApp>();
    
    // Register routes
    registerRoutes();
}

void TelemetryServer::registerRoutes() {
    // Register REST API routes
    m_restApi->registerRoutes(*m_app);
    
    // Register WebSocket routes
    m_webSocketApi->registerRoutes(*m_app);
    
    // Add a simple health check endpoint
    CROW_ROUTE((*m_app), "/health").methods("GET"_method)
        ([]() {
            crow::json::wvalue response;
            response["status"] = "ok";
            response["service"] = "telemetry-server";
            return crow::response(200, response);
        });
    
    // Add a root endpoint with basic info
    CROW_ROUTE((*m_app), "/").methods("GET"_method)
        ([this]() {
            crow::json::wvalue response;
            response["service"] = "Telemetry Server";
            response["version"] = "1.0.0";
            response["endpoints"] = {
                {"rest", "/api/*"},
                {"websocket", "/ws"}
            };
            response["connected_clients"] = getConnectedClientCount();
            return crow::response(200, response);
        });
}

void TelemetryServer::start(const std::string& bind_address) {
    if (m_running) {
        std::cerr << "Server is already running" << std::endl;
        return;
    }
    
    // Set up signal handlers for graceful shutdown
    g_server = this;
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    m_running = true;
    
    // Run the server in a separate thread
    m_serverThread = std::thread([this, bind_address]() {
        std::cout << "Starting Telemetry Server on " << bind_address << ":" << m_port << std::endl;
        std::cout << "REST API: http://" << bind_address << ":" << m_port << "/api" << std::endl;
        std::cout << "WebSocket: ws://" << bind_address << ":" << m_port << "/ws" << std::endl;
        std::cout << "Health check: http://" << bind_address << ":" << m_port << "/health" << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        m_app->bindaddr(bind_address).port(m_port).multithreaded().run();
    });
    
    // Wait a bit for the server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void TelemetryServer::stop() {
    if (!m_running) {
        return;
    }
    
    std::cout << "Stopping Telemetry Server..." << std::endl;
    
    m_running = false;
    
    // Stop the Crow app
    if (m_app) {
        m_app->stop();
    }
    
    // Wait for the server thread to finish
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
    
    // Clear all WebSocket connections
    if (m_wsService) {
        m_wsService->clearAllConnections();
    }
    
    std::cout << "Telemetry Server stopped" << std::endl;
}

bool TelemetryServer::isRunning() const {
    return m_running;
}

size_t TelemetryServer::getConnectedClientCount() const {
    if (m_wsService) {
        return m_wsService->getConnectedClientCount();
    }
    return 0;
}

} // namespace telemetry
