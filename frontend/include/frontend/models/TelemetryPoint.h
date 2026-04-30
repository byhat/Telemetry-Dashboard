#ifndef FRONTEND_MODELS_TELEMETRYPOINT_H
#define FRONTEND_MODELS_TELEMETRYPOINT_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

namespace frontend {
namespace models {

struct TelemetryPoint {
    QString deviceId;
    double temperature;
    double power;
    double voltage;
    QDateTime timestamp;
    
    TelemetryPoint();
    TelemetryPoint(const QString& id, double temp, double pwr, double volt, const QDateTime& ts);
    
    QString toJson() const;
    static TelemetryPoint fromJson(const QString& json);
    static TelemetryPoint fromJsonObject(const QJsonObject& obj);
};

} // namespace models
} // namespace frontend

#endif // FRONTEND_MODELS_TELEMETRYPOINT_H
