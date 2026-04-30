#pragma once

#include "telemetry/storage/IDataStorage.h"
#include "telemetry/models/TelemetryData.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <mutex>

namespace telemetry {
namespace services {

/**
 * @brief Service for handling telemetry data operations
 * 
 * This service provides business logic for telemetry data processing,
 * including validation, storage, and notification of registered callbacks.
 * It integrates with the storage layer and supports real-time updates
 * through callback registration.
 */
class TelemetryService {
public:
    /**
     * @brief Callback type for telemetry updates
     */
    using TelemetryCallback = std::function<void(const models::TelemetryData&)>;
    
    /**
     * @brief Constructor
     * @param storage Pointer to the storage implementation
     */
    explicit TelemetryService(std::shared_ptr<storage::IDataStorage> storage);
    
    /**
     * @brief Destructor
     */
    ~TelemetryService();
    
    // Disable copy and move
    TelemetryService(const TelemetryService&) = delete;
    TelemetryService& operator=(const TelemetryService&) = delete;
    TelemetryService(TelemetryService&&) = delete;
    TelemetryService& operator=(TelemetryService&&) = delete;
    
    /**
     * @brief Process a single telemetry data point
     * @param data The telemetry data to process
     * @return true if processing was successful, false otherwise
     */
    bool processTelemetry(const models::TelemetryData& data);
    
    /**
     * @brief Process multiple telemetry data points in a batch
     * @param data Vector of telemetry data to process
     * @return Number of successfully processed data points
     */
    size_t processTelemetryBatch(const std::vector<models::TelemetryData>& data);
    
    /**
     * @brief Get the latest telemetry data for a specific device
     * @param device_id The device identifier
     * @return Optional containing the latest telemetry, or nullopt if not found
     */
    std::optional<models::TelemetryData> getLatestTelemetry(const std::string& device_id);
    
    /**
     * @brief Get telemetry history for a specific device
     * @param device_id The device identifier
     * @param limit Maximum number of records to return (0 = all)
     * @return Vector of telemetry data, ordered by timestamp (newest first)
     */
    std::vector<models::TelemetryData> getTelemetryHistory(const std::string& device_id, size_t limit = 0);
    
    /**
     * @brief Get all unique device IDs
     * @return Vector of device IDs
     */
    std::vector<std::string> getAllDeviceIds();
    
    /**
     * @brief Get all telemetry data across all devices
     * @return Vector of all telemetry data
     */
    std::vector<models::TelemetryData> getAllTelemetry();
    
    /**
     * @brief Register a callback for telemetry updates
     * @param callback The callback function to invoke when new telemetry is received
     */
    void registerTelemetryCallback(TelemetryCallback callback);
    
    /**
     * @brief Unregister the telemetry callback
     */
    void unregisterTelemetryCallback();
    
    /**
     * @brief Validate telemetry data
     * @param data The telemetry data to validate
     * @return true if the data is valid, false otherwise
     */
    bool validateTelemetry(const models::TelemetryData& data);
    
    /**
     * @brief Get telemetry statistics for a specific device
     * @param device_id The device identifier
     * @return Optional containing the statistics, or nullopt if not available
     */
    std::optional<storage::IDataStorage::TelemetryStats> getTelemetryStats(const std::string& device_id);
    
private:
    std::shared_ptr<storage::IDataStorage> m_storage;
    TelemetryCallback m_callback;
    mutable std::mutex m_callbackMutex;
    
    /**
     * @brief Notify registered callback about new telemetry data
     * @param data The telemetry data to notify about
     */
    void notifyCallback(const models::TelemetryData& data);
};

} // namespace services
} // namespace telemetry
