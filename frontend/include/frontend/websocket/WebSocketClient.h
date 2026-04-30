#ifndef FRONTEND_WEBSOCKET_WEBSOCKETCLIENT_H
#define FRONTEND_WEBSOCKET_WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QString>
#include <QTimer>

namespace frontend {
namespace websocket {

class WebSocketClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectionStateChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    
public:
    explicit WebSocketClient(QObject* parent = nullptr);
    ~WebSocketClient();
    
    // Connection management
    bool connected() const;
    QString serverUrl() const;
    void setServerUrl(const QString& url);
    
    Q_INVOKABLE void connectToServer();
    Q_INVOKABLE void disconnectFromServer();
    
    // Message handling
    Q_INVOKABLE void sendMessage(const QString& message);
    
    // Configuration
    void setReconnectEnabled(bool enabled);
    bool reconnectEnabled() const;
    void setReconnectInterval(int milliseconds);
    int reconnectInterval() const;
    
signals:
    void connectionStateChanged();
    void serverUrlChanged();
    void messageReceived(const QString& message);
    void telemetryReceived(const QString& deviceId, double temperature, double power, double voltage);
    void errorReceived(const QString& deviceId, const QString& errorText);
    void connectionError(const QString& error);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimeout();
    
private:
    void processMessage(const QString& message);
    
    QWebSocket* m_webSocket;
    QString m_serverUrl;
    bool m_reconnectEnabled;
    int m_reconnectInterval;
    QTimer* m_reconnectTimer;
};

} // namespace websocket
} // namespace frontend

#endif // FRONTEND_WEBSOCKET_WEBSOCKETCLIENT_H
