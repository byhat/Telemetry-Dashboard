#ifndef FRONTEND_MODELS_DEVICEDATA_H
#define FRONTEND_MODELS_DEVICEDATA_H

#include "frontend/models/TelemetryPoint.h"
#include "frontend/models/ErrorEntry.h"
#include <QString>
#include <QVector>
#include <QVariantList>
#include <QVariantMap>

namespace frontend {
namespace models {

class DeviceData {
public:
    explicit DeviceData(const QString& deviceId);
    
    QString deviceId() const;
    
    // Telemetry data
    void addTelemetryPoint(const TelemetryPoint& point);
    QVector<TelemetryPoint> telemetryHistory() const;
    TelemetryPoint latestTelemetry() const;
    
    // Error data
    void addErrorEntry(const ErrorEntry& entry);
    QVector<ErrorEntry> errorHistory() const;
    
    // Data management
    void clearTelemetry();
    void clearErrors();
    void clearAll();
    
    // Limits
    void setMaxTelemetryPoints(int max);
    int maxTelemetryPoints() const;
    
    void setMaxErrorEntries(int max);
    int maxErrorEntries() const;
    
    // Data access for QML (formatted as QVariantList)
    QVariantList getTemperatureSeries() const;
    QVariantList getPowerSeries() const;
    QVariantList getVoltageSeries() const;
    QVariantList getErrorSeries() const;
    QVariantList getErrorHistoryList() const;
    
    // Latest values
    QVariantMap getLatestValues() const;
    
private:
    QString m_deviceId;
    QVector<TelemetryPoint> m_telemetryHistory;
    QVector<ErrorEntry> m_errorHistory;
    int m_maxTelemetryPoints;
    int m_maxErrorEntries;
};

} // namespace models
} // namespace frontend

#endif // FRONTEND_MODELS_DEVICEDATA_H
