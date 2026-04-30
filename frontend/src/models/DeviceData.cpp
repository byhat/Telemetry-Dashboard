#include "frontend/models/DeviceData.h"

namespace frontend {
namespace models {

DeviceData::DeviceData(const QString& deviceId)
    : m_deviceId(deviceId)
    , m_maxTelemetryPoints(1000)
    , m_maxErrorEntries(100)
{
}

QString DeviceData::deviceId() const
{
    return m_deviceId;
}

void DeviceData::addTelemetryPoint(const TelemetryPoint& point)
{
    m_telemetryHistory.append(point);
    
    // Enforce circular buffer behavior
    while (m_telemetryHistory.size() > m_maxTelemetryPoints) {
        m_telemetryHistory.removeFirst();
    }
}

QVector<TelemetryPoint> DeviceData::telemetryHistory() const
{
    return m_telemetryHistory;
}

TelemetryPoint DeviceData::latestTelemetry() const
{
    if (m_telemetryHistory.isEmpty()) {
        return TelemetryPoint();
    }
    return m_telemetryHistory.last();
}

void DeviceData::addErrorEntry(const ErrorEntry& entry)
{
    m_errorHistory.append(entry);
    
    // Enforce circular buffer behavior
    while (m_errorHistory.size() > m_maxErrorEntries) {
        m_errorHistory.removeFirst();
    }
}

QVector<ErrorEntry> DeviceData::errorHistory() const
{
    return m_errorHistory;
}

void DeviceData::clearTelemetry()
{
    m_telemetryHistory.clear();
}

void DeviceData::clearErrors()
{
    m_errorHistory.clear();
}

void DeviceData::clearAll()
{
    clearTelemetry();
    clearErrors();
}

void DeviceData::setMaxTelemetryPoints(int max)
{
    if (max > 0) {
        m_maxTelemetryPoints = max;
        // Trim existing data if needed
        while (m_telemetryHistory.size() > m_maxTelemetryPoints) {
            m_telemetryHistory.removeFirst();
        }
    }
}

int DeviceData::maxTelemetryPoints() const
{
    return m_maxTelemetryPoints;
}

void DeviceData::setMaxErrorEntries(int max)
{
    if (max > 0) {
        m_maxErrorEntries = max;
        // Trim existing data if needed
        while (m_errorHistory.size() > m_maxErrorEntries) {
            m_errorHistory.removeFirst();
        }
    }
}

int DeviceData::maxErrorEntries() const
{
    return m_maxErrorEntries;
}

QVariantList DeviceData::getTemperatureSeries() const
{
    QVariantList series;
    for (const auto& point : m_telemetryHistory) {
        QVariantMap dataPoint;
        dataPoint["timestamp"] = point.timestamp.toMSecsSinceEpoch();
        dataPoint["value"] = point.temperature;
        series.append(dataPoint);
    }
    return series;
}

QVariantList DeviceData::getPowerSeries() const
{
    QVariantList series;
    for (const auto& point : m_telemetryHistory) {
        QVariantMap dataPoint;
        dataPoint["timestamp"] = point.timestamp.toMSecsSinceEpoch();
        dataPoint["value"] = point.power;
        series.append(dataPoint);
    }
    return series;
}

QVariantList DeviceData::getVoltageSeries() const
{
    QVariantList series;
    for (const auto& point : m_telemetryHistory) {
        QVariantMap dataPoint;
        dataPoint["timestamp"] = point.timestamp.toMSecsSinceEpoch();
        dataPoint["value"] = point.voltage;
        series.append(dataPoint);
    }
    return series;
}

QVariantList DeviceData::getErrorSeries() const
{
    QVariantList series;
    for (const auto& entry : m_errorHistory) {
        QVariantMap dataPoint;
        dataPoint["timestamp"] = entry.timestamp.toMSecsSinceEpoch();
        dataPoint["value"] = 1; // Each error counts as 1
        series.append(dataPoint);
    }
    return series;
}

QVariantList DeviceData::getErrorHistoryList() const
{
    QVariantList list;
    for (const auto& entry : m_errorHistory) {
        QVariantMap entryMap;
        entryMap["timestamp"] = entry.timestamp.toString(Qt::ISODate);
        entryMap["errorText"] = entry.errorText;
        list.append(entryMap);
    }
    return list;
}

QVariantMap DeviceData::getLatestValues() const
{
    QVariantMap values;
    TelemetryPoint latest = latestTelemetry();
    
    values["temperature"] = latest.temperature;
    values["power"] = latest.power;
    values["voltage"] = latest.voltage;
    values["timestamp"] = latest.timestamp.toString(Qt::ISODate);
    
    return values;
}

} // namespace models
} // namespace frontend
