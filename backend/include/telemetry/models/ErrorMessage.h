#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <crow/json.h>

namespace telemetry {
namespace models {

/**
 * @brief Error message structure for storing device errors
 * 
 * This struct holds error message data including device ID, error text,
 * and timestamp. It includes JSON serialization methods for integration
 * with the Crow web framework.
 */
struct ErrorMessage {
    std::string device_id;
    std::string error_text;
    std::chrono::system_clock::time_point timestamp;
    
    /**
     * @brief Default constructor
     */
    ErrorMessage();
    
    /**
     * @brief Constructor with all fields except timestamp
     * @param id Device identifier
     * @param error Error message text
     */
    ErrorMessage(const std::string& id, const std::string& error);
    
    /**
     * @brief Full constructor with explicit timestamp
     * @param id Device identifier
     * @param error Error message text
     * @param ts Timestamp
     */
    ErrorMessage(const std::string& id, const std::string& error,
                 const std::chrono::system_clock::time_point& ts);
    
    /**
     * @brief Convert timestamp to Unix epoch milliseconds
     * @return Timestamp as milliseconds since Unix epoch
     */
    int64_t getTimestampMs() const;
    
    /**
     * @brief Set timestamp from Unix epoch milliseconds
     * @param ms Milliseconds since Unix epoch
     */
    void setTimestampMs(int64_t ms);
};

} // namespace models
} // namespace telemetry

// JSON serialization for Crow framework
namespace telemetry {
namespace models {

inline crow::json::wvalue errorMessageToJson(const ErrorMessage& data) {
    crow::json::wvalue r;
    r["device_id"] = data.device_id;
    r["error"] = data.error_text;
    r["timestamp"] = data.getTimestampMs();
    return r;
}

inline ErrorMessage jsonToErrorMessage(const crow::json::rvalue& r) {
    ErrorMessage data;
    data.device_id = r["device_id"].s();
    data.error_text = r["error"].s();
    data.setTimestampMs(r["timestamp"].i());
    return data;
}

} // namespace models
} // namespace telemetry
