#pragma once

#include "telemetry/storage/IDataStorage.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <algorithm>

namespace telemetry {
namespace storage {

/**
 * @brief In-memory implementation of IDataStorage
 * 
 * This class provides a thread-safe in-memory storage implementation for
 * telemetry and error data. It uses unordered maps for efficient lookups
 * and vectors for storing data per device. Data retention is implemented
 * to limit memory usage by keeping only the most recent N data points.
 */
class InMemoryStorage : public IDataStorage {
public:
    /**
     * @brief Constructor
     * @param max_points_per_device Maximum number of telemetry points to keep per device
     * @param max_errors_per_device Maximum number of errors to keep per device
     */
    explicit InMemoryStorage(size_t max_points_per_device = 1000,
                             size_t max_errors_per_device = 100);
    
    /**
     * @brief Destructor
     */
    ~InMemoryStorage() override;
    
    void storeTelemetry(const models::TelemetryData& data) override;
    void storeTelemetryBatch(const std::vector<models::TelemetryData>& data) override;
    std::optional<models::TelemetryData> getLatestTelemetry(const std::string& device_id) override;
    std::vector<models::TelemetryData> getTelemetryHistory(const std::string& device_id, 
                                                           size_t limit = 0) override;
    std::vector<models::TelemetryData> getTelemetryHistoryInRange(
        const std::string& device_id,
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end
    ) override;
    std::vector<models::TelemetryData> getAllTelemetry() override;
    std::vector<std::string> getAllDeviceIds() override;
    
    void storeError(const models::ErrorMessage& error) override;
    void storeErrorBatch(const std::vector<models::ErrorMessage>& errors) override;
    std::vector<models::ErrorMessage> getErrorHistory(const std::string& device_id, 
                                                      size_t limit = 0) override;
    std::vector<models::ErrorMessage> getAllErrors() override;
    std::vector<models::ErrorMessage> getLatestErrors(size_t limit = 10) override;
    
    std::optional<TelemetryStats> getTelemetryStats(const std::string& device_id) override;
    
    void clearAll() override;
    void pruneOldData(const std::chrono::system_clock::time_point& cutoff) override;
    
    /**
     * @brief Set the maximum number of telemetry points per device
     * @param max_points Maximum points to retain
     */
    void setMaxPointsPerDevice(size_t max_points);
    
    /**
     * @brief Set the maximum number of errors per device
     * @param max_errors Maximum errors to retain
     */
    void setMaxErrorsPerDevice(size_t max_errors);
    
    /**
     * @brief Get the current number of telemetry points for a device
     * @param device_id The device identifier
     * @return Number of telemetry points stored
     */
    size_t getTelemetryCount(const std::string& device_id) const;
    
    /**
     * @brief Get the current number of errors for a device
     * @param device_id The device identifier
     * @return Number of errors stored
     */
    size_t getErrorCount(const std::string& device_id) const;
    
private:
    // Storage containers
    std::unordered_map<std::string, std::vector<models::TelemetryData>> m_telemetryData;
    std::unordered_map<std::string, std::vector<models::ErrorMessage>> m_errorData;
    
    // Thread safety
    mutable std::mutex m_mutex;
    
    // Configuration
    size_t m_maxPointsPerDevice;
    size_t m_maxErrorsPerDevice;
    
    /**
     * @brief Add telemetry data to history and enforce retention policy
     * @param data The telemetry data to add
     */
    void addTelemetryToHistory(const models::TelemetryData& data);
    
    /**
     * @brief Add error message to history and enforce retention policy
     * @param error The error message to add
     */
    void addErrorToHistory(const models::ErrorMessage& error);
    
    /**
     * @brief Trim telemetry data for a device to max_points
     * @param device_id The device identifier
     */
    void trimTelemetryHistory(const std::string& device_id);
    
    /**
     * @brief Trim error data for a device to max_errors
     * @param device_id The device identifier
     */
    void trimErrorHistory(const std::string& device_id);
};

} // namespace storage
} // namespace telemetry
