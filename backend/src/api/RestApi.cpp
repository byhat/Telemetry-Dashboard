#include "telemetry/api/RestApi.h"
#include "telemetry/models/TelemetryData.h"
#include "telemetry/models/ErrorMessage.h"
#include <sstream>
#include <chrono>

namespace telemetry {
namespace api {

RestApi::RestApi(std::shared_ptr<services::TelemetryService> telemetry_service,
                 std::shared_ptr<services::ErrorService> error_service)
    : m_telemetryService(std::move(telemetry_service))
    , m_errorService(std::move(error_service))
{
}

crow::json::wvalue RestApi::createSuccessResponse(const crow::json::wvalue& data) {
    crow::json::wvalue response;
    response["success"] = true;
    response["data"] = crow::json::wvalue(data);
    return response;
}

crow::json::wvalue RestApi::createErrorResponse(const std::string& error_message, 
                                                 const std::string& error_code) {
    crow::json::wvalue response;
    response["success"] = false;
    response["error"] = error_message;
    response["code"] = error_code;
    return response;
}

crow::response RestApi::handlePostTelemetry(const crow::request& req) {
    try {
        auto json_data = crow::json::load(req.body);
        if (!json_data) {
            return crow::response(400, createErrorResponse("Invalid JSON", "INVALID_JSON"));
        }
        
        models::TelemetryData data = models::jsonToTelemetryData(json_data);
        
        if (m_telemetryService->processTelemetry(data)) {
            return crow::response(200, createSuccessResponse(crow::json::wvalue()));
        } else {
            return crow::response(400, createErrorResponse("Invalid telemetry data", "INVALID_DATA"));
        }
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handlePostError(const crow::request& req) {
    try {
        auto json_data = crow::json::load(req.body);
        if (!json_data) {
            return crow::response(400, createErrorResponse("Invalid JSON", "INVALID_JSON"));
        }
        
        models::ErrorMessage error = models::jsonToErrorMessage(json_data);
        
        if (m_errorService->processError(error)) {
            return crow::response(200, createSuccessResponse(crow::json::wvalue()));
        } else {
            return crow::response(400, createErrorResponse("Invalid error data", "INVALID_DATA"));
        }
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetDevices(const crow::request& req) {
    try {
        auto devices = m_telemetryService->getAllDeviceIds();
        
        crow::json::wvalue data;
        crow::json::wvalue device_list(crow::json::type::List);
        
        for (size_t i = 0; i < devices.size(); ++i) {
            device_list[i] = devices[i];
        }
        
        data = std::move(device_list);
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetLatestTelemetry(const crow::request& req, const std::string& device_id) {
    try {
        auto telemetry = m_telemetryService->getLatestTelemetry(device_id);
        
        if (!telemetry) {
            return crow::response(404, createErrorResponse("Device not found or no data available", "DEVICE_NOT_FOUND"));
        }
        
        crow::json::wvalue data = models::telemetryDataToJson(*telemetry);
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetTelemetryHistory(const crow::request& req, const std::string& device_id) {
    try {
        // Parse limit query parameter
        size_t limit = 0;
        auto limit_param = req.url_params.get("limit");
        if (limit_param) {
            try {
                limit = std::stoul(limit_param);
            } catch (...) {
                // Invalid limit, use default
            }
        }
        
        auto history = m_telemetryService->getTelemetryHistory(device_id, limit);
        
        crow::json::wvalue data(crow::json::type::List);
        for (size_t i = 0; i < history.size(); ++i) {
            data[i] = models::telemetryDataToJson(history[i]);
        }
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetDeviceErrors(const crow::request& req, const std::string& device_id) {
    try {
        // Parse limit query parameter
        size_t limit = 0;
        auto limit_param = req.url_params.get("limit");
        if (limit_param) {
            try {
                limit = std::stoul(limit_param);
            } catch (...) {
                // Invalid limit, use default
            }
        }
        
        auto errors = m_errorService->getErrorHistory(device_id, limit);
        
        crow::json::wvalue data(crow::json::type::List);
        for (size_t i = 0; i < errors.size(); ++i) {
            data[i] = models::errorMessageToJson(errors[i]);
        }
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetLatestErrors(const crow::request& req) {
    try {
        // Parse limit query parameter
        size_t limit = 10;
        auto limit_param = req.url_params.get("limit");
        if (limit_param) {
            try {
                limit = std::stoul(limit_param);
            } catch (...) {
                // Invalid limit, use default
            }
        }
        
        auto errors = m_errorService->getLatestErrors(limit);
        
        crow::json::wvalue data(crow::json::type::List);
        for (size_t i = 0; i < errors.size(); ++i) {
            data[i] = models::errorMessageToJson(errors[i]);
        }
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetDeviceStats(const crow::request& req, const std::string& device_id) {
    try {
        auto stats = m_telemetryService->getTelemetryStats(device_id);
        
        if (!stats) {
            return crow::response(404, createErrorResponse("Device not found or no data available", "DEVICE_NOT_FOUND"));
        }
        
        crow::json::wvalue data;
        data["temperature"]["avg"] = stats->avg_temperature;
        data["temperature"]["min"] = stats->min_temperature;
        data["temperature"]["max"] = stats->max_temperature;
        data["power"]["avg"] = stats->avg_power;
        data["power"]["min"] = stats->min_power;
        data["power"]["max"] = stats->max_power;
        data["voltage"]["avg"] = stats->avg_voltage;
        data["voltage"]["min"] = stats->min_voltage;
        data["voltage"]["max"] = stats->max_voltage;
        data["count"] = stats->count;
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

crow::response RestApi::handleGetAllTelemetry(const crow::request& req) {
    try {
        // Parse limit query parameter
        size_t limit = 0;
        auto limit_param = req.url_params.get("limit");
        if (limit_param) {
            try {
                limit = std::stoul(limit_param);
            } catch (...) {
                // Invalid limit, use default (0 = all)
            }
        }
        
        auto all_telemetry = m_telemetryService->getAllTelemetry();
        
        // If limit is specified and > 0, slice the result
        if (limit > 0 && all_telemetry.size() > limit) {
            all_telemetry.resize(limit);
        }
        
        crow::json::wvalue data(crow::json::type::List);
        for (size_t i = 0; i < all_telemetry.size(); ++i) {
            data[i] = models::telemetryDataToJson(all_telemetry[i]);
        }
        
        return crow::response(200, createSuccessResponse(data));
    } catch (const std::exception& e) {
        return crow::response(500, createErrorResponse(e.what(), "INTERNAL_ERROR"));
    }
}

void RestApi::registerRoutes(crow::SimpleApp& app) {
    // Handle OPTIONS requests for CORS preflight
    CROW_ROUTE(app, "/api/<string>")
        .methods("OPTIONS"_method)
        ([](const std::string& path) {
            (void)path;  // Suppress unused parameter warning
            crow::response res(204);  // No Content
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            return res;
        });
    
    // POST /api/telemetry - receive telemetry data
    CROW_ROUTE(app, "/api/telemetry").methods("POST"_method)
        ([this](const crow::request& req) {
            auto res = handlePostTelemetry(req);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/telemetry/all - get all telemetry data across all devices
    CROW_ROUTE(app, "/api/telemetry/all").methods("GET"_method)
        ([this](const crow::request& req) {
            auto res = handleGetAllTelemetry(req);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // POST /api/error - receive error messages
    CROW_ROUTE(app, "/api/error").methods("POST"_method)
        ([this](const crow::request& req) {
            auto res = handlePostError(req);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/devices - get list of all device IDs
    CROW_ROUTE(app, "/api/devices").methods("GET"_method)
        ([this](const crow::request& req) {
            auto res = handleGetDevices(req);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/devices/<device_id>/telemetry/latest - get latest telemetry
    CROW_ROUTE(app, "/api/devices/<string>/telemetry/latest").methods("GET"_method)
        ([this](const crow::request& req, const std::string& device_id) {
            auto res = handleGetLatestTelemetry(req, device_id);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/devices/<device_id>/telemetry/history - get telemetry history
    CROW_ROUTE(app, "/api/devices/<string>/telemetry/history").methods("GET"_method)
        ([this](const crow::request& req, const std::string& device_id) {
            auto res = handleGetTelemetryHistory(req, device_id);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/devices/<device_id>/errors - get error history for a device
    CROW_ROUTE(app, "/api/devices/<string>/errors").methods("GET"_method)
        ([this](const crow::request& req, const std::string& device_id) {
            auto res = handleGetDeviceErrors(req, device_id);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/errors/latest - get latest errors across all devices
    CROW_ROUTE(app, "/api/errors/latest").methods("GET"_method)
        ([this](const crow::request& req) {
            auto res = handleGetLatestErrors(req);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
    
    // GET /api/devices/<device_id>/stats - get telemetry statistics
    CROW_ROUTE(app, "/api/devices/<string>/stats").methods("GET"_method)
        ([this](const crow::request& req, const std::string& device_id) {
            auto res = handleGetDeviceStats(req, device_id);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        });
}

} // namespace api
} // namespace telemetry
