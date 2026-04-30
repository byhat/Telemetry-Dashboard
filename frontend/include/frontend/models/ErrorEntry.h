#ifndef FRONTEND_MODELS_ERRORENTRY_H
#define FRONTEND_MODELS_ERRORENTRY_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

namespace frontend {
namespace models {

struct ErrorEntry {
    QString deviceId;
    QString errorText;
    QDateTime timestamp;
    
    ErrorEntry();
    ErrorEntry(const QString& id, const QString& error, const QDateTime& ts);
    
    QString toJson() const;
    static ErrorEntry fromJson(const QString& json);
    static ErrorEntry fromJsonObject(const QJsonObject& obj);
};

} // namespace models
} // namespace frontend

#endif // FRONTEND_MODELS_ERRORENTRY_H
