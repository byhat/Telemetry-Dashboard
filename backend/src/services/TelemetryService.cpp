#include "telemetry/services/TelemetryService.h"
#include <mutex>

namespace telemetry {
namespace services {

TelemetryService::TelemetryService(std::shared_ptr<storage::IDataStorage> storage)
    : m_storage(std::move(storage))
{
}

TelemetryService::~TelemetryService() = default;

bool TelemetryService::validateTelemetry(const models::TelemetryData& data) {
    // Validate device_id
    if (data.device_id.empty() || data.device_id.length() > 255) {
        return false;
    }
    
    // Validate temperature range (-50 to 150 Celsius)
    if (data.temperature < -50.0 || data.temperature > 150.0) {
        return false;
    }
    
    // Validate power range (0 to 1000 Watts)
    if (data.power < 0.0 || data.power > 1000.0) {
        return false;
    }
    
    // Validate voltage range (0.1 to 250 Volts)
    if (data.voltage < 0.1 || data.voltage > 250.0) {
        return false;
    }
    
    return true;
}

bool TelemetryService::processTelemetry(const models::TelemetryData& data) {
    if (!validateTelemetry(data)) {
        return false;
    }
    
    // Store the data
    m_storage->storeTelemetry(data);
    
    // Notify callback
    notifyCallback(data);
    
    return true;
}

size_t TelemetryService::processTelemetryBatch(const std::vector<models::TelemetryData>& data) {
    std::vector<models::TelemetryData> valid_data;
    valid_data.reserve(data.size());
    
    for (const auto& item : data) {
        if (validateTelemetry(item)) {
            valid_data.push_back(item);
        }
    }
    
    if (valid_data.empty()) {
        return 0;
    }
    
    // Store the batch
    m_storage->storeTelemetryBatch(valid_data);
    
    // Notify callback for each item
    for (const auto& item : valid_data) {
        notifyCallback(item);
    }
    
    return valid_data.size();
}

std::optional<models::TelemetryData> TelemetryService::getLatestTelemetry(const std::string& device_id) {
    return m_storage->getLatestTelemetry(device_id);
}

std::vector<models::TelemetryData> TelemetryService::getTelemetryHistory(const std::string& device_id, size_t limit) {
    return m_storage->getTelemetryHistory(device_id, limit);
}

std::vector<std::string> TelemetryService::getAllDeviceIds() {
    return m_storage->getAllDeviceIds();
}

std::vector<models::TelemetryData> TelemetryService::getAllTelemetry() {
    return m_storage->getAllTelemetry();
}

void TelemetryService::registerTelemetryCallback(TelemetryCallback callback) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callback = std::move(callback);
}

void TelemetryService::unregisterTelemetryCallback() {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callback = nullptr;
}

void TelemetryService::notifyCallback(const models::TelemetryData& data) {
    TelemetryCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        callback = m_callback;
    }
    
    if (callback) {
        callback(data);
    }
}

std::optional<storage::IDataStorage::TelemetryStats> TelemetryService::getTelemetryStats(const std::string& device_id) {
    return m_storage->getTelemetryStats(device_id);
}

} // namespace services
} // namespace telemetry
