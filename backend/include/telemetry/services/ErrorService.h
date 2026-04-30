#pragma once

#include "telemetry/storage/IDataStorage.h"
#include "telemetry/models/ErrorMessage.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <mutex>

namespace telemetry {
namespace services {

/**
 * @brief Service for handling error message operations
 * 
 * This service provides business logic for error message processing,
 * including validation, storage, and notification of registered callbacks.
 * It integrates with the storage layer and supports real-time updates
 * through callback registration.
 */
class ErrorService {
public:
    /**
     * @brief Callback type for error updates
     */
    using ErrorCallback = std::function<void(const models::ErrorMessage&)>;
    
    /**
     * @brief Constructor
     * @param storage Pointer to the storage implementation
     */
    explicit ErrorService(std::shared_ptr<storage::IDataStorage> storage);
    
    /**
     * @brief Destructor
     */
    ~ErrorService();
    
    // Disable copy and move
    ErrorService(const ErrorService&) = delete;
    ErrorService& operator=(const ErrorService&) = delete;
    ErrorService(ErrorService&&) = delete;
    ErrorService& operator=(ErrorService&&) = delete;
    
    /**
     * @brief Process a single error message
     * @param error The error message to process
     * @return true if processing was successful, false otherwise
     */
    bool processError(const models::ErrorMessage& error);
    
    /**
     * @brief Process multiple error messages in a batch
     * @param errors Vector of error messages to process
     * @return Number of successfully processed error messages
     */
    size_t processErrorBatch(const std::vector<models::ErrorMessage>& errors);
    
    /**
     * @brief Get error history for a specific device
     * @param device_id The device identifier
     * @param limit Maximum number of records to return (0 = all)
     * @return Vector of error messages, ordered by timestamp (newest first)
     */
    std::vector<models::ErrorMessage> getErrorHistory(const std::string& device_id, size_t limit = 0);
    
    /**
     * @brief Get the latest errors across all devices
     * @param limit Maximum number of records to return (default: 10)
     * @return Vector of error messages, ordered by timestamp (newest first)
     */
    std::vector<models::ErrorMessage> getLatestErrors(size_t limit = 10);
    
    /**
     * @brief Get all error messages across all devices
     * @return Vector of all error messages
     */
    std::vector<models::ErrorMessage> getAllErrors();
    
    /**
     * @brief Register a callback for error updates
     * @param callback The callback function to invoke when new errors are received
     */
    void registerErrorCallback(ErrorCallback callback);
    
    /**
     * @brief Unregister the error callback
     */
    void unregisterErrorCallback();
    
    /**
     * @brief Validate an error message
     * @param error The error message to validate
     * @return true if the error message is valid, false otherwise
     */
    bool validateError(const models::ErrorMessage& error);
    
private:
    std::shared_ptr<storage::IDataStorage> m_storage;
    ErrorCallback m_callback;
    mutable std::mutex m_callbackMutex;
    
    /**
     * @brief Notify registered callback about a new error message
     * @param error The error message to notify about
     */
    void notifyCallback(const models::ErrorMessage& error);
};

} // namespace services
} // namespace telemetry
