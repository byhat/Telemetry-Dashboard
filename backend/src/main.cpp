#include "telemetry/TelemetryServer.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    // Parse command line arguments
    int port = 18080;
    size_t max_telemetry_points = 1000;
    size_t max_error_points = 100;
    std::string bind_address = "0.0.0.0";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--max-telemetry" && i + 1 < argc) {
            max_telemetry_points = std::stoul(argv[++i]);
        } else if (arg == "--max-errors" && i + 1 < argc) {
            max_error_points = std::stoul(argv[++i]);
        } else if (arg == "--bind" && i + 1 < argc) {
            bind_address = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Telemetry Server - Real-time telemetry data collection and broadcasting\n\n"
                      << "Usage: " << argv[0] << " [OPTIONS]\n\n"
                      << "Options:\n"
                      << "  --port PORT              Port number to listen on (default: 18080)\n"
                      << "  --bind ADDRESS           Address to bind to (default: 0.0.0.0)\n"
                      << "  --max-telemetry POINTS   Max telemetry points per device (default: 1000)\n"
                      << "  --max-errors POINTS      Max error points per device (default: 100)\n"
                      << "  --help, -h               Show this help message\n\n"
                      << "API Endpoints:\n"
                      << "  REST API:    http://" << bind_address << ":" << port << "/api\n"
                      << "  WebSocket:   ws://" << bind_address << ":" << port << "/ws\n"
                      << "  Health:      http://" << bind_address << ":" << port << "/health\n\n"
                      << "Examples:\n"
                      << "  " << argv[0] << "                          # Start with defaults\n"
                      << "  " << argv[0] << " --port 8080              # Use port 8080\n"
                      << "  " << argv[0] << " --bind 127.0.0.1         # Bind to localhost only\n"
                      << std::endl;
            return 0;
        }
    }
    
    try {
        // Create and configure the server
        auto server = std::make_unique<telemetry::TelemetryServer>(
            port, max_telemetry_points, max_error_points
        );
        
        // Start the server
        server->start(bind_address);
        
        // Keep the main thread alive
        while (server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
