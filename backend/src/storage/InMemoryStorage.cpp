#include "telemetry/storage/InMemoryStorage.h"
#include <algorithm>
#include <numeric>
#include <limits>

namespace telemetry {
namespace storage {

InMemoryStorage::InMemoryStorage(size_t max_points_per_device, size_t max_errors_per_device)
    : m_maxPointsPerDevice(max_points_per_device)
    , m_maxErrorsPerDevice(max_errors_per_device)
{
}

InMemoryStorage::~InMemoryStorage() = default;

void InMemoryStorage::storeTelemetry(const models::TelemetryData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    addTelemetryToHistory(data);
}

void InMemoryStorage::storeTelemetryBatch(const std::vector<models::TelemetryData>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& item : data) {
        addTelemetryToHistory(item);
    }
}

std::optional<models::TelemetryData> InMemoryStorage::getLatestTelemetry(const std::string& device_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_telemetryData.find(device_id);
    if (it == m_telemetryData.end() || it->second.empty()) {
        return std::nullopt;
    }
    
    // Data is stored in chronological order, so the last element is the latest
    return it->second.back();
}

std::vector<models::TelemetryData> InMemoryStorage::getTelemetryHistory(
    const std::string& device_id, size_t limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_telemetryData.find(device_id);
    if (it == m_telemetryData.end()) {
        return {};
    }
    
    const auto& history = it->second;
    if (limit == 0 || limit >= history.size()) {
        // Return in reverse order (newest first)
        std::vector<models::TelemetryData> result(history.rbegin(), history.rend());
        return result;
    }
    
    // Return the last 'limit' elements in reverse order
    std::vector<models::TelemetryData> result(history.end() - limit, history.end());
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<models::TelemetryData> InMemoryStorage::getTelemetryHistoryInRange(
    const std::string& device_id,
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_telemetryData.find(device_id);
    if (it == m_telemetryData.end()) {
        return {};
    }
    
    std::vector<models::TelemetryData> result;
    for (const auto& data : it->second) {
        if (data.timestamp >= start && data.timestamp <= end) {
            result.push_back(data);
        }
    }
    
    // Return in reverse order (newest first)
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<models::TelemetryData> InMemoryStorage::getAllTelemetry() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<models::TelemetryData> result;
    for (const auto& pair : m_telemetryData) {
        result.insert(result.end(), pair.second.begin(), pair.second.end());
    }
    
    // Sort by timestamp (newest first)
    std::sort(result.begin(), result.end(),
        [](const models::TelemetryData& a, const models::TelemetryData& b) {
            return a.timestamp > b.timestamp;
        });
    
    return result;
}

std::vector<std::string> InMemoryStorage::getAllDeviceIds() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::string> result;
    result.reserve(m_telemetryData.size());
    
    for (const auto& pair : m_telemetryData) {
        result.push_back(pair.first);
    }
    
    return result;
}

void InMemoryStorage::storeError(const models::ErrorMessage& error) {
    std::lock_guard<std::mutex> lock(m_mutex);
    addErrorToHistory(error);
}

void InMemoryStorage::storeErrorBatch(const std::vector<models::ErrorMessage>& errors) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& item : errors) {
        addErrorToHistory(item);
    }
}

std::vector<models::ErrorMessage> InMemoryStorage::getErrorHistory(
    const std::string& device_id, size_t limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_errorData.find(device_id);
    if (it == m_errorData.end()) {
        return {};
    }
    
    const auto& history = it->second;
    if (limit == 0 || limit >= history.size()) {
        // Return in reverse order (newest first)
        std::vector<models::ErrorMessage> result(history.rbegin(), history.rend());
        return result;
    }
    
    // Return the last 'limit' elements in reverse order
    std::vector<models::ErrorMessage> result(history.end() - limit, history.end());
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<models::ErrorMessage> InMemoryStorage::getAllErrors() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<models::ErrorMessage> result;
    for (const auto& pair : m_errorData) {
        result.insert(result.end(), pair.second.begin(), pair.second.end());
    }
    
    // Sort by timestamp (newest first)
    std::sort(result.begin(), result.end(),
        [](const models::ErrorMessage& a, const models::ErrorMessage& b) {
            return a.timestamp > b.timestamp;
        });
    
    return result;
}

std::vector<models::ErrorMessage> InMemoryStorage::getLatestErrors(size_t limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<models::ErrorMessage> allErrors = getAllErrors();
    
    if (limit >= allErrors.size()) {
        return allErrors;
    }
    
    return std::vector<models::ErrorMessage>(allErrors.begin(), 
                                             allErrors.begin() + limit);
}

std::optional<InMemoryStorage::TelemetryStats> InMemoryStorage::getTelemetryStats(
    const std::string& device_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_telemetryData.find(device_id);
    if (it == m_telemetryData.end() || it->second.empty()) {
        return std::nullopt;
    }
    
    const auto& data = it->second;
    TelemetryStats stats;
    
    // Initialize min/max values
    stats.min_temperature = std::numeric_limits<double>::max();
    stats.max_temperature = std::numeric_limits<double>::lowest();
    stats.min_power = std::numeric_limits<double>::max();
    stats.max_power = std::numeric_limits<double>::lowest();
    stats.min_voltage = std::numeric_limits<double>::max();
    stats.max_voltage = std::numeric_limits<double>::lowest();
    stats.count = data.size();
    
    double sum_temp = 0.0;
    double sum_power = 0.0;
    double sum_voltage = 0.0;
    
    for (const auto& point : data) {
        // Update sums
        sum_temp += point.temperature;
        sum_power += point.power;
        sum_voltage += point.voltage;
        
        // Update min/max
        stats.min_temperature = std::min(stats.min_temperature, point.temperature);
        stats.max_temperature = std::max(stats.max_temperature, point.temperature);
        stats.min_power = std::min(stats.min_power, point.power);
        stats.max_power = std::max(stats.max_power, point.power);
        stats.min_voltage = std::min(stats.min_voltage, point.voltage);
        stats.max_voltage = std::max(stats.max_voltage, point.voltage);
    }
    
    // Calculate averages
    stats.avg_temperature = sum_temp / stats.count;
    stats.avg_power = sum_power / stats.count;
    stats.avg_voltage = sum_voltage / stats.count;
    
    return stats;
}

void InMemoryStorage::clearAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_telemetryData.clear();
    m_errorData.clear();
}

void InMemoryStorage::pruneOldData(const std::chrono::system_clock::time_point& cutoff) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Prune telemetry data
    for (auto& pair : m_telemetryData) {
        auto& history = pair.second;
        auto new_end = std::remove_if(history.begin(), history.end(),
            [&cutoff](const models::TelemetryData& data) {
                return data.timestamp < cutoff;
            });
        history.erase(new_end, history.end());
    }
    
    // Remove empty telemetry entries
    for (auto it = m_telemetryData.begin(); it != m_telemetryData.end(); ) {
        if (it->second.empty()) {
            it = m_telemetryData.erase(it);
        } else {
            ++it;
        }
    }
    
    // Prune error data
    for (auto& pair : m_errorData) {
        auto& history = pair.second;
        auto new_end = std::remove_if(history.begin(), history.end(),
            [&cutoff](const models::ErrorMessage& error) {
                return error.timestamp < cutoff;
            });
        history.erase(new_end, history.end());
    }
    
    // Remove empty error entries
    for (auto it = m_errorData.begin(); it != m_errorData.end(); ) {
        if (it->second.empty()) {
            it = m_errorData.erase(it);
        } else {
            ++it;
        }
    }
}

void InMemoryStorage::setMaxPointsPerDevice(size_t max_points) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxPointsPerDevice = max_points;
    
    // Trim existing data to new limit
    for (const auto& pair : m_telemetryData) {
        trimTelemetryHistory(pair.first);
    }
}

void InMemoryStorage::setMaxErrorsPerDevice(size_t max_errors) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxErrorsPerDevice = max_errors;
    
    // Trim existing data to new limit
    for (const auto& pair : m_errorData) {
        trimErrorHistory(pair.first);
    }
}

size_t InMemoryStorage::getTelemetryCount(const std::string& device_id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_telemetryData.find(device_id);
    if (it == m_telemetryData.end()) {
        return 0;
    }
    return it->second.size();
}

size_t InMemoryStorage::getErrorCount(const std::string& device_id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_errorData.find(device_id);
    if (it == m_errorData.end()) {
        return 0;
    }
    return it->second.size();
}

void InMemoryStorage::addTelemetryToHistory(const models::TelemetryData& data) {
    auto& history = m_telemetryData[data.device_id];
    history.push_back(data);
    trimTelemetryHistory(data.device_id);
}

void InMemoryStorage::addErrorToHistory(const models::ErrorMessage& error) {
    auto& history = m_errorData[error.device_id];
    history.push_back(error);
    trimErrorHistory(error.device_id);
}

void InMemoryStorage::trimTelemetryHistory(const std::string& device_id) {
    auto it = m_telemetryData.find(device_id);
    if (it == m_telemetryData.end()) {
        return;
    }
    
    auto& history = it->second;
    if (history.size() > m_maxPointsPerDevice) {
        // Remove oldest elements (front of vector)
        size_t to_remove = history.size() - m_maxPointsPerDevice;
        history.erase(history.begin(), history.begin() + to_remove);
    }
}

void InMemoryStorage::trimErrorHistory(const std::string& device_id) {
    auto it = m_errorData.find(device_id);
    if (it == m_errorData.end()) {
        return;
    }
    
    auto& history = it->second;
    if (history.size() > m_maxErrorsPerDevice) {
        // Remove oldest elements (front of vector)
        size_t to_remove = history.size() - m_maxErrorsPerDevice;
        history.erase(history.begin(), history.begin() + to_remove);
    }
}

} // namespace storage
} // namespace telemetry
