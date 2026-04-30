#include "telemetry/services/ErrorService.h"
#include <mutex>

namespace telemetry {
namespace services {

ErrorService::ErrorService(std::shared_ptr<storage::IDataStorage> storage)
    : m_storage(std::move(storage))
{
}

ErrorService::~ErrorService() = default;

bool ErrorService::validateError(const models::ErrorMessage& error) {
    // Validate device_id
    if (error.device_id.empty() || error.device_id.length() > 255) {
        return false;
    }
    
    // Validate error_text
    if (error.error_text.empty() || error.error_text.length() > 1000) {
        return false;
    }
    
    return true;
}

bool ErrorService::processError(const models::ErrorMessage& error) {
    if (!validateError(error)) {
        return false;
    }
    
    // Store the error
    m_storage->storeError(error);
    
    // Notify callback
    notifyCallback(error);
    
    return true;
}

size_t ErrorService::processErrorBatch(const std::vector<models::ErrorMessage>& errors) {
    std::vector<models::ErrorMessage> valid_errors;
    valid_errors.reserve(errors.size());
    
    for (const auto& item : errors) {
        if (validateError(item)) {
            valid_errors.push_back(item);
        }
    }
    
    if (valid_errors.empty()) {
        return 0;
    }
    
    // Store the batch
    m_storage->storeErrorBatch(valid_errors);
    
    // Notify callback for each item
    for (const auto& item : valid_errors) {
        notifyCallback(item);
    }
    
    return valid_errors.size();
}

std::vector<models::ErrorMessage> ErrorService::getErrorHistory(const std::string& device_id, size_t limit) {
    return m_storage->getErrorHistory(device_id, limit);
}

std::vector<models::ErrorMessage> ErrorService::getLatestErrors(size_t limit) {
    return m_storage->getLatestErrors(limit);
}

std::vector<models::ErrorMessage> ErrorService::getAllErrors() {
    return m_storage->getAllErrors();
}

void ErrorService::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callback = std::move(callback);
}

void ErrorService::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callback = nullptr;
}

void ErrorService::notifyCallback(const models::ErrorMessage& error) {
    ErrorCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        callback = m_callback;
    }
    
    if (callback) {
        callback(error);
    }
}

} // namespace services
} // namespace telemetry
