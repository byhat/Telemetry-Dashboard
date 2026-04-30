#include "frontend/models/ErrorEntry.h"
#include <QJsonParseError>

namespace frontend {
namespace models {

ErrorEntry::ErrorEntry()
    : timestamp(QDateTime::currentDateTime())
{
}

ErrorEntry::ErrorEntry(const QString& id, const QString& error, const QDateTime& ts)
    : deviceId(id)
    , errorText(error)
    , timestamp(ts)
{
}

QString ErrorEntry::toJson() const
{
    QJsonObject obj;
    obj["device_id"] = deviceId;
    obj["error_text"] = errorText;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

ErrorEntry ErrorEntry::fromJson(const QString& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return ErrorEntry();
    }
    
    return fromJsonObject(doc.object());
}

ErrorEntry ErrorEntry::fromJsonObject(const QJsonObject& obj)
{
    ErrorEntry entry;
    entry.deviceId = obj["device_id"].toString();
    entry.errorText = obj["error_text"].toString();
    
    QString timestampStr = obj["timestamp"].toString();
    if (!timestampStr.isEmpty()) {
        entry.timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    }
    
    return entry;
}

} // namespace models
} // namespace frontend
