#pragma once

#include "telemetry/models/TelemetryData.h"
#include "telemetry/models/ErrorMessage.h"
#include <vector>
#include <string>
#include <optional>
#include <chrono>

namespace telemetry {
namespace storage {

/**
 * @brief Pure virtual interface for data storage operations
 * 
 * This interface defines the contract for all storage implementations,
 * enabling dependency inversion and easy replacement of storage backends.
 * Implementations must ensure thread safety for concurrent access.
 */
class IDataStorage {
public:
    virtual ~IDataStorage() = default;
    
    /**
     * @brief Store a single telemetry data point
     * @param data The telemetry data to store
     */
    virtual void storeTelemetry(const models::TelemetryData& data) = 0;
    
    /**
     * @brief Store multiple telemetry data points in a batch
     * @param data Vector of telemetry data to store
     */
    virtual void storeTelemetryBatch(const std::vector<models::TelemetryData>& data) = 0;
    
    /**
     * @brief Get the latest telemetry data for a specific device
     * @param device_id The device identifier
     * @return Optional containing the latest telemetry, or nullopt if not found
     */
    virtual std::optional<models::TelemetryData> getLatestTelemetry(const std::string& device_id) = 0;
    
    /**
     * @brief Get telemetry history for a specific device
     * @param device_id The device identifier
     * @param limit Maximum number of records to return (0 = all)
     * @return Vector of telemetry data, ordered by timestamp (newest first)
     */
    virtual std::vector<models::TelemetryData> getTelemetryHistory(const std::string& device_id, 
                                                                   size_t limit = 0) = 0;
    
    /**
     * @brief Get telemetry history within a specific time range
     * @param device_id The device identifier
     * @param start Start of time range (inclusive)
     * @param end End of time range (inclusive)
     * @return Vector of telemetry data within the time range
     */
    virtual std::vector<models::TelemetryData> getTelemetryHistoryInRange(
        const std::string& device_id,
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end
    ) = 0;
    
    /**
     * @brief Get all telemetry data across all devices
     * @return Vector of all telemetry data
     */
    virtual std::vector<models::TelemetryData> getAllTelemetry() = 0;
    
    /**
     * @brief Get all unique device IDs
     * @return Vector of device IDs
     */
    virtual std::vector<std::string> getAllDeviceIds() = 0;
    
    /**
     * @brief Store a single error message
     * @param error The error message to store
     */
    virtual void storeError(const models::ErrorMessage& error) = 0;
    
    /**
     * @brief Store multiple error messages in a batch
     * @param errors Vector of error messages to store
     */
    virtual void storeErrorBatch(const std::vector<models::ErrorMessage>& errors) = 0;
    
    /**
     * @brief Get error history for a specific device
     * @param device_id The device identifier
     * @param limit Maximum number of records to return (0 = all)
     * @return Vector of error messages, ordered by timestamp (newest first)
     */
    virtual std::vector<models::ErrorMessage> getErrorHistory(const std::string& device_id, 
                                                              size_t limit = 0) = 0;
    
    /**
     * @brief Get all error messages across all devices
     * @return Vector of all error messages
     */
    virtual std::vector<models::ErrorMessage> getAllErrors() = 0;
    
    /**
     * @brief Get the latest error messages across all devices
     * @param limit Maximum number of errors to return
     * @return Vector of latest error messages
     */
    virtual std::vector<models::ErrorMessage> getLatestErrors(size_t limit = 10) = 0;
    
    /**
     * @brief Statistics structure for telemetry data
     */
    struct TelemetryStats {
        double avg_temperature;
        double avg_power;
        double avg_voltage;
        double min_temperature;
        double max_temperature;
        double min_power;
        double max_power;
        double min_voltage;
        double max_voltage;
        size_t count;
    };
    
    /**
     * @brief Get telemetry statistics for a specific device
     * @param device_id The device identifier
     * @return Optional containing statistics, or nullopt if no data found
     */
    virtual std::optional<TelemetryStats> getTelemetryStats(const std::string& device_id) = 0;
    
    /**
     * @brief Clear all stored data (telemetry and errors)
     */
    virtual void clearAll() = 0;
    
    /**
     * @brief Remove data older than the specified cutoff time
     * @param cutoff Cutoff time - data older than this will be removed
     */
    virtual void pruneOldData(const std::chrono::system_clock::time_point& cutoff) = 0;
};

} // namespace storage
} // namespace telemetry
