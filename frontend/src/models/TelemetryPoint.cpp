#include "frontend/models/TelemetryPoint.h"
#include <QJsonParseError>

namespace frontend {
namespace models {

TelemetryPoint::TelemetryPoint()
    : temperature(0.0)
    , power(0.0)
    , voltage(0.0)
    , timestamp(QDateTime::currentDateTime())
{
}

TelemetryPoint::TelemetryPoint(const QString& id, double temp, double pwr, double volt, const QDateTime& ts)
    : deviceId(id)
    , temperature(temp)
    , power(pwr)
    , voltage(volt)
    , timestamp(ts)
{
}

QString TelemetryPoint::toJson() const
{
    QJsonObject obj;
    obj["device_id"] = deviceId;
    obj["temperature"] = temperature;
    obj["power"] = power;
    obj["voltage"] = voltage;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

TelemetryPoint TelemetryPoint::fromJson(const QString& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return TelemetryPoint();
    }
    
    return fromJsonObject(doc.object());
}

TelemetryPoint TelemetryPoint::fromJsonObject(const QJsonObject& obj)
{
    TelemetryPoint point;
    point.deviceId = obj["device_id"].toString();
    point.temperature = obj["temperature"].toDouble(0.0);
    point.power = obj["power"].toDouble(0.0);
    point.voltage = obj["voltage"].toDouble(0.0);
    
    QString timestampStr = obj["timestamp"].toString();
    if (!timestampStr.isEmpty()) {
        point.timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    }
    
    return point;
}

} // namespace models
} // namespace frontend
