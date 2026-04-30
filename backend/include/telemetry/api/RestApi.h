#pragma once

#include "telemetry/services/TelemetryService.h"
#include "telemetry/services/ErrorService.h"
#include <crow.h>
#include <memory>

namespace telemetry {
namespace api {

/**
 * @brief REST API handler for telemetry endpoints
 * 
 * This class provides REST API endpoints for the telemetry system,
 * including telemetry data submission, retrieval, and error management.
 * It integrates with the service layer to process requests and return responses.
 */
class RestApi {
public:
    /**
     * @brief Constructor
     * @param telemetry_service Pointer to the telemetry service
     * @param error_service Pointer to the error service
     */
    RestApi(std::shared_ptr<services::TelemetryService> telemetry_service,
            std::shared_ptr<services::ErrorService> error_service);
    
    /**
     * @brief Destructor
     */
    ~RestApi() = default;
    
    /**
     * @brief Register all API routes with the Crow application
     * @param app Reference to the Crow application
     */
    void registerRoutes(crow::SimpleApp& app);
    
private:
    std::shared_ptr<services::TelemetryService> m_telemetryService;
    std::shared_ptr<services::ErrorService> m_errorService;
    
    /**
     * @brief Create a success response
     * @param data The data to include in the response
     * @return Crow JSON response
     */
    crow::json::wvalue createSuccessResponse(const crow::json::wvalue& data);
    
    /**
     * @brief Create an error response
     * @param error_message The error message
     * @param error_code The error code
     * @return Crow JSON response
     */
    crow::json::wvalue createErrorResponse(const std::string& error_message, 
                                           const std::string& error_code = "ERROR");
    
    /**
     * @brief Handle POST /api/telemetry - receive telemetry data
     */
    crow::response handlePostTelemetry(const crow::request& req);
    
    /**
     * @brief Handle POST /api/error - receive error messages
     */
    crow::response handlePostError(const crow::request& req);
    
    /**
     * @brief Handle GET /api/devices - get list of all device IDs
     */
    crow::response handleGetDevices(const crow::request& req);
    
    /**
     * @brief Handle GET /api/devices/<device_id>/telemetry/latest - get latest telemetry
     */
    crow::response handleGetLatestTelemetry(const crow::request& req, const std::string& device_id);
    
    /**
     * @brief Handle GET /api/devices/<device_id>/telemetry/history - get telemetry history
     */
    crow::response handleGetTelemetryHistory(const crow::request& req, const std::string& device_id);
    
    /**
     * @brief Handle GET /api/devices/<device_id>/errors - get error history for a device
     */
    crow::response handleGetDeviceErrors(const crow::request& req, const std::string& device_id);
    
    /**
     * @brief Handle GET /api/errors/latest - get latest errors across all devices
     */
    crow::response handleGetLatestErrors(const crow::request& req);
    
    /**
     * @brief Handle GET /api/devices/<device_id>/stats - get telemetry statistics
     */
    crow::response handleGetDeviceStats(const crow::request& req, const std::string& device_id);
    
    /**
     * @brief Handle GET /api/telemetry/all - get all telemetry data across all devices
     */
    crow::response handleGetAllTelemetry(const crow::request& req);
};

} // namespace api
} // namespace telemetry
