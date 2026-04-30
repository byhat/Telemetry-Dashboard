#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <crow/json.h>

namespace telemetry {
namespace models {

/**
 * @brief Telemetry data structure for storing device measurements
 * 
 * This struct holds telemetry data including temperature, power, voltage,
 * and timestamp information. It includes JSON serialization methods for
 * integration with the Crow web framework.
 */
struct TelemetryData {
    std::string device_id;
    double temperature;      // Temperature in Celsius
    double power;           // Power in Watts
    double voltage;         // Voltage in Volts
    std::chrono::system_clock::time_point timestamp;
    
    /**
     * @brief Default constructor
     */
    TelemetryData();
    
    /**
     * @brief Constructor with all fields except timestamp
     * @param id Device identifier
     * @param temp Temperature in Celsius
     * @param pwr Power in Watts
     * @param volt Voltage in Volts
     */
    TelemetryData(const std::string& id, double temp, double pwr, double volt);
    
    /**
     * @brief Full constructor with explicit timestamp
     * @param id Device identifier
     * @param temp Temperature in Celsius
     * @param pwr Power in Watts
     * @param volt Voltage in Volts
     * @param ts Timestamp
     */
    TelemetryData(const std::string& id, double temp, double pwr, double volt,
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

inline crow::json::wvalue telemetryDataToJson(const TelemetryData& data) {
    crow::json::wvalue r;
    r["device_id"] = data.device_id;
    r["temperature"] = data.temperature;
    r["power"] = data.power;
    r["voltage"] = data.voltage;
    r["timestamp"] = data.getTimestampMs();
    return r;
}

inline TelemetryData jsonToTelemetryData(const crow::json::rvalue& r) {
    TelemetryData data;
    data.device_id = r["device_id"].s();
    data.temperature = r["temperature"].d();
    data.power = r["power"].d();
    data.voltage = r["voltage"].d();
    data.setTimestampMs(r["timestamp"].i());
    return data;
}

} // namespace models
} // namespace telemetry
