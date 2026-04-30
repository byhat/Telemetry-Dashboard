#include "telemetry/models/ErrorMessage.h"
#include <chrono>

namespace telemetry {
namespace models {

ErrorMessage::ErrorMessage()
    : device_id("")
    , error_text("")
    , timestamp(std::chrono::system_clock::now())
{
}

ErrorMessage::ErrorMessage(const std::string& id, const std::string& error)
    : device_id(id)
    , error_text(error)
    , timestamp(std::chrono::system_clock::now())
{
}

ErrorMessage::ErrorMessage(const std::string& id, const std::string& error,
                           const std::chrono::system_clock::time_point& ts)
    : device_id(id)
    , error_text(error)
    , timestamp(ts)
{
}

int64_t ErrorMessage::getTimestampMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()
    ).count();
}

void ErrorMessage::setTimestampMs(int64_t ms) {
    timestamp = std::chrono::system_clock::time_point(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::milliseconds(ms)
        )
    );
}

} // namespace models
} // namespace telemetry
