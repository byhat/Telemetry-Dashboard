#ifndef FRONTEND_VIEWMODELS_TELEMETRYVIEWMODEL_H
#define FRONTEND_VIEWMODELS_TELEMETRYVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "frontend/websocket/WebSocketClient.h"
#include "frontend/models/DeviceData.h"

namespace frontend {
namespace viewmodels {

class TelemetryViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList deviceIds READ deviceIds NOTIFY deviceIdsChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice WRITE setSelectedDevice NOTIFY selectedDeviceChanged)
    Q_PROPERTY(int dataUpdateCounter READ dataUpdateCounter NOTIFY dataUpdateCounterChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(QString connectionError READ connectionError WRITE setConnectionError NOTIFY connectionErrorChanged)
    
public:
    explicit TelemetryViewModel(QObject* parent = nullptr);
    ~TelemetryViewModel();
    
    // Connection management
    bool connected() const;
    QString serverUrl() const;
    void setServerUrl(const QString& url);
    QString serverAddress() const;
    void setServerAddress(const QString& address);
    int serverPort() const;
    void setServerPort(int port);
    QString connectionError() const;
    void setConnectionError(const QString& error);
    
    Q_INVOKABLE void connectToServer();
    Q_INVOKABLE void disconnectFromServer();
    
    // Device management
    QVariantList deviceIds() const;
    QString selectedDevice() const;
    void setSelectedDevice(const QString& deviceId);
    int dataUpdateCounter() const { return m_dataUpdateCounter; }
    
    // Data access for currently selected device
    // Renamed from getSelectedDeviceTemperatureData to match Q_PROPERTY READ specifier
    QVariantList selectedDeviceTemperatureData() const;
    QVariantList selectedDevicePowerData() const;
    QVariantList selectedDeviceVoltageData() const;
    
    // Telemetry data access
    Q_INVOKABLE QVariantList getTemperatureData(const QString& deviceId) const;
    Q_INVOKABLE QVariantList getPowerData(const QString& deviceId) const;
    Q_INVOKABLE QVariantList getVoltageData(const QString& deviceId) const;
    Q_INVOKABLE QVariantMap getLatestTelemetry(const QString& deviceId) const;
    
    // Data access for currently selected device (used by QML bindings)
    // Note: For READ to work correctly with QML, the getter must be named exactly as specified
    Q_PROPERTY(QVariantList selectedDeviceTemperatureData READ selectedDeviceTemperatureData NOTIFY dataUpdateCounterChanged)
    Q_PROPERTY(QVariantList selectedDevicePowerData READ selectedDevicePowerData NOTIFY dataUpdateCounterChanged)
    Q_PROPERTY(QVariantList selectedDeviceVoltageData READ selectedDeviceVoltageData NOTIFY dataUpdateCounterChanged)
    
    // Error data access
    Q_INVOKABLE QVariantList getErrorHistory(const QString& deviceId) const;
    Q_INVOKABLE QVariantList getErrorData(const QString& deviceId) const;
    Q_INVOKABLE QVariantList getRecentErrors(int limit = 10) const;
    
    // Data management
    Q_INVOKABLE void clearDeviceData(const QString& deviceId);
    Q_INVOKABLE void clearAllData();
    
    // Configuration
    Q_INVOKABLE void setMaxHistoryPoints(int max);
    Q_INVOKABLE void setMaxErrorEntries(int max);
    
    // Device existence check
    Q_INVOKABLE bool hasDevice(const QString& deviceId) const;
    
    // Historical data fetching
    Q_INVOKABLE void fetchHistoricalData();
    
signals:
    void deviceIdsChanged();
    void selectedDeviceChanged();
    void dataUpdateCounterChanged();
    void connectionStatusChanged();
    void serverUrlChanged();
    void serverAddressChanged();
    void serverPortChanged();
    void connectionErrorChanged();
    void telemetryDataUpdated(const QString& deviceId);
    void errorDataUpdated(const QString& deviceId);
    void newErrorReceived(const QString& deviceId, const QString& errorText);
    
public slots:
    void onTelemetryReceived(const QString& deviceId, double temperature, double power, double voltage);
    void onErrorReceived(const QString& deviceId, const QString& errorText);
    
private slots:
    void onHistoricalDataReceived(QNetworkReply* reply);
    void onConnectionStateChanged();
    
private:
    void ensureDeviceExists(const QString& deviceId);
    void updateDeviceIds();
    void updateServerUrl();
    void onConnectionError(const QString& error);
    
    websocket::WebSocketClient* m_webSocket;
    QNetworkAccessManager* m_networkManager;
    QMap<QString, models::DeviceData> m_devices;
    QString m_selectedDevice;
    QString m_serverAddress;
    int m_serverPort;
    QString m_connectionError;
    int m_maxHistoryPoints;
    int m_maxErrorEntries;
    int m_dataUpdateCounter = 0;
};

} // namespace viewmodels
} // namespace frontend

#endif // FRONTEND_VIEWMODELS_TELEMETRYVIEWMODEL_H
