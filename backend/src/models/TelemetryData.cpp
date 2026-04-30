#include "telemetry/models/TelemetryData.h"
#include <chrono>

namespace telemetry {
namespace models {

TelemetryData::TelemetryData()
    : device_id("")
    , temperature(0.0)
    , power(0.0)
    , voltage(0.0)
    , timestamp(std::chrono::system_clock::now())
{
}

TelemetryData::TelemetryData(const std::string& id, double temp, double pwr, double volt)
    : device_id(id)
    , temperature(temp)
    , power(pwr)
    , voltage(volt)
    , timestamp(std::chrono::system_clock::now())
{
}

TelemetryData::TelemetryData(const std::string& id, double temp, double pwr, double volt,
                             const std::chrono::system_clock::time_point& ts)
    : device_id(id)
    , temperature(temp)
    , power(pwr)
    , voltage(volt)
    , timestamp(ts)
{
}

int64_t TelemetryData::getTimestampMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()
    ).count();
}

void TelemetryData::setTimestampMs(int64_t ms) {
    timestamp = std::chrono::system_clock::time_point(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::milliseconds(ms)
        )
    );
}

} // namespace models
} // namespace telemetry
