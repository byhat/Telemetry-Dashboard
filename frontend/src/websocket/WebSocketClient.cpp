#include "frontend/websocket/WebSocketClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

namespace frontend {
namespace websocket {

WebSocketClient::WebSocketClient(QObject* parent)
    : QObject(parent)
    , m_webSocket(new QWebSocket())
    , m_serverUrl("ws://localhost:18080/ws")
    , m_reconnectEnabled(true)
    , m_reconnectInterval(5000)
    , m_reconnectTimer(new QTimer(this))
{
    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this, &WebSocketClient::onError);
    
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::onReconnectTimeout);
}

WebSocketClient::~WebSocketClient()
{
    disconnectFromServer();
    m_webSocket->deleteLater();
}

bool WebSocketClient::connected() const
{
    return m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState;
}

QString WebSocketClient::serverUrl() const
{
    return m_serverUrl;
}

void WebSocketClient::setServerUrl(const QString& url)
{
    if (m_serverUrl != url) {
        m_serverUrl = url;
        emit serverUrlChanged();
    }
}

void WebSocketClient::connectToServer()
{
    qDebug() << "=== WebSocketClient::connectToServer() CALLED ===";
    qDebug() << "m_webSocket pointer:" << (void*)m_webSocket;
    qDebug() << "m_serverUrl:" << m_serverUrl;
    
    if (!m_webSocket) {
        qCritical() << "ERROR: m_webSocket is NULL!";
        return;
    }
    
    QAbstractSocket::SocketState currentState = m_webSocket->state();
    qDebug() << "Current WebSocket state:" << currentState;
    
    if (currentState == QAbstractSocket::ConnectedState) {
        qDebug() << "Already connected, returning early";
        return;
    }
    
    qDebug() << "Stopping reconnect timer";
    m_reconnectTimer->stop();
    
    qDebug() << "Opening WebSocket connection to:" << m_serverUrl;
    m_webSocket->open(QUrl(m_serverUrl));
    qDebug() << "WebSocket open() called";
    qDebug() << "=== WebSocketClient::connectToServer() END ===";
}

void WebSocketClient::disconnectFromServer()
{
    m_reconnectTimer->stop();
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->close();
    }
}

void WebSocketClient::sendMessage(const QString& message)
{
    if (connected()) {
        m_webSocket->sendTextMessage(message);
    } else {
        qWarning() << "Cannot send message: not connected to server";
    }
}

void WebSocketClient::setReconnectEnabled(bool enabled)
{
    m_reconnectEnabled = enabled;
}

bool WebSocketClient::reconnectEnabled() const
{
    return m_reconnectEnabled;
}

void WebSocketClient::setReconnectInterval(int milliseconds)
{
    if (milliseconds > 0) {
        m_reconnectInterval = milliseconds;
    }
}

int WebSocketClient::reconnectInterval() const
{
    return m_reconnectInterval;
}

void WebSocketClient::onConnected()
{
    qDebug() << "=== WebSocketClient::onConnected() ===";
    qDebug() << "WebSocket successfully connected to" << m_serverUrl;
    qDebug() << "Current state:" << m_webSocket->state();
    m_reconnectTimer->stop();
    emit connectionStateChanged();
    qDebug() << "=== WebSocketClient::onConnected() END ===";
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "=== WebSocketClient::onDisconnected() ===";
    qDebug() << "WebSocket disconnected from" << m_serverUrl;
    qDebug() << "Current state:" << m_webSocket->state();
    emit connectionStateChanged();
    
    // Attempt reconnection if enabled
    if (m_reconnectEnabled) {
        qDebug() << "Attempting to reconnect in" << m_reconnectInterval << "ms";
        m_reconnectTimer->start(m_reconnectInterval);
    }
    qDebug() << "=== WebSocketClient::onDisconnected() END ===";
}

void WebSocketClient::onTextMessageReceived(const QString& message)
{
    qDebug() << "=== WebSocketClient::onTextMessageReceived() ===";
    qDebug() << "Message received:" << message;
    emit messageReceived(message);
    processMessage(message);
    qDebug() << "=== WebSocketClient::onTextMessageReceived() END ===";
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "=== WebSocketClient::onError() ===";
    QString errorString = m_webSocket->errorString();
    qDebug() << "Socket error code:" << error;
    qDebug() << "Socket error string:" << errorString;
    qDebug() << "Current state:" << m_webSocket->state();
    qWarning() << "WebSocket error:" << errorString;
    emit connectionError(errorString);
    qDebug() << "=== WebSocketClient::onError() END ===";
}

void WebSocketClient::onReconnectTimeout()
{
    qDebug() << "Attempting to reconnect...";
    connectToServer();
}

void WebSocketClient::processMessage(const QString& message)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON message:" << parseError.errorString();
        return;
    }
    
    if (!doc.isObject()) {
        qWarning() << "Message is not a JSON object";
        return;
    }
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    
    if (type == "telemetry" || type == "telemetry_update") {
        QString deviceId = obj["device_id"].toString();
        double temperature = obj["temperature"].toDouble(0.0);
        double power = obj["power"].toDouble(0.0);
        double voltage = obj["voltage"].toDouble(0.0);
        
        emit telemetryReceived(deviceId, temperature, power, voltage);
        qDebug() << "Received telemetry for device" << deviceId
                 << "T:" << temperature << "P:" << power << "V:" << voltage;
    }
    else if (type == "error" || type == "error_update") {
        QString deviceId = obj["device_id"].toString();
        QString errorText = obj["error_text"].toString();
        
        emit errorReceived(deviceId, errorText);
        qDebug() << "Received error for device" << deviceId << ":" << errorText;
    }
    else if (type == "status") {
        // Status messages are welcome/heartbeat messages, just log them
        QString message = obj["message"].toString();
        qDebug() << "Status message:" << message;
    }
    else {
        qWarning() << "Unknown message type:" << type;
    }
}

} // namespace websocket
} // namespace frontend
