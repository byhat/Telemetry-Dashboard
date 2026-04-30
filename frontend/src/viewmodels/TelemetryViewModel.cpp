#include "frontend/viewmodels/TelemetryViewModel.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

namespace frontend {
namespace viewmodels {

TelemetryViewModel::TelemetryViewModel(QObject* parent)
    : QObject(parent)
    , m_webSocket(new websocket::WebSocketClient(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_serverAddress("localhost")
    , m_serverPort(18080)
    , m_maxHistoryPoints(1000)
    , m_maxErrorEntries(100)
{
    // Connect WebSocket signals to ViewModel slots
    connect(m_webSocket, &websocket::WebSocketClient::telemetryReceived,
            this, &TelemetryViewModel::onTelemetryReceived);
    connect(m_webSocket, &websocket::WebSocketClient::errorReceived,
            this, &TelemetryViewModel::onErrorReceived);
    connect(m_webSocket, &websocket::WebSocketClient::connectionStateChanged,
            this, &TelemetryViewModel::onConnectionStateChanged);
    connect(m_webSocket, &websocket::WebSocketClient::connectionError,
            this, &TelemetryViewModel::onConnectionError);
    
    // Connect network manager finished signal
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &TelemetryViewModel::onHistoricalDataReceived);
    
    // Initialize with default URL
    updateServerUrl();
}

TelemetryViewModel::~TelemetryViewModel()
{
    disconnectFromServer();
}

bool TelemetryViewModel::connected() const
{
    return m_webSocket->connected();
}

QString TelemetryViewModel::serverUrl() const
{
    return m_webSocket->serverUrl();
}

void TelemetryViewModel::setServerUrl(const QString& url)
{
    m_webSocket->setServerUrl(url);
    emit serverUrlChanged();
}

QString TelemetryViewModel::serverAddress() const
{
    return m_serverAddress;
}

void TelemetryViewModel::setServerAddress(const QString& address)
{
    if (m_serverAddress != address) {
        m_serverAddress = address;
        emit serverAddressChanged();
        updateServerUrl();
    }
}

int TelemetryViewModel::serverPort() const
{
    return m_serverPort;
}

void TelemetryViewModel::setServerPort(int port)
{
    if (m_serverPort != port) {
        m_serverPort = port;
        emit serverPortChanged();
        updateServerUrl();
    }
}

QString TelemetryViewModel::connectionError() const
{
    return m_connectionError;
}

void TelemetryViewModel::setConnectionError(const QString& error)
{
    if (m_connectionError != error) {
        m_connectionError = error;
        emit connectionErrorChanged();
    }
}

void TelemetryViewModel::updateServerUrl()
{
    QString url = QString("ws://%1:%2/ws").arg(m_serverAddress).arg(m_serverPort);
    m_webSocket->setServerUrl(url);
    emit serverUrlChanged();
    qDebug() << "Server URL updated to:" << url;
}

void TelemetryViewModel::onConnectionStateChanged()
{
    emit connectionStatusChanged();
    
    // Fetch historical data when WebSocket connects
    if (m_webSocket->connected()) {
        qDebug() << "WebSocket connected, fetching historical data...";
        fetchHistoricalData();
    }
}

void TelemetryViewModel::onConnectionError(const QString& error)
{
    setConnectionError(error);
    qWarning() << "Connection error:" << error;
}

void TelemetryViewModel::connectToServer()
{
    qDebug() << "=== TelemetryViewModel::connectToServer() CALLED ===";
    qDebug() << "m_webSocket pointer:" << (void*)m_webSocket;
    qDebug() << "m_serverAddress:" << m_serverAddress;
    qDebug() << "m_serverPort:" << m_serverPort;
    qDebug() << "WebSocket URL:" << m_webSocket->serverUrl();
    qDebug() << "WebSocket connected():" << m_webSocket->connected();
    qDebug() << "Calling m_webSocket->connectToServer()...";
    m_webSocket->connectToServer();
    qDebug() << "=== TelemetryViewModel::connectToServer() END ===";
}

void TelemetryViewModel::disconnectFromServer()
{
    m_webSocket->disconnectFromServer();
}

QVariantList TelemetryViewModel::deviceIds() const
{
    QVariantList list;
    for (const QString& deviceId : m_devices.keys()) {
        list.append(deviceId);
    }
    return list;
}

QString TelemetryViewModel::selectedDevice() const
{
    return m_selectedDevice;
}

void TelemetryViewModel::setSelectedDevice(const QString& deviceId)
{
    if (m_selectedDevice != deviceId) {
        m_selectedDevice = deviceId;
        emit selectedDeviceChanged();
    }
}

QVariantList TelemetryViewModel::getTemperatureData(const QString& deviceId) const
{
    qDebug() << "=== getTemperatureData() CALLED ===";
    qDebug() << "  deviceId:" << deviceId;
    qDebug() << "  dataUpdateCounter:" << m_dataUpdateCounter;
    qDebug() << "  Total devices in map:" << m_devices.size();
    qDebug() << "  Device IDs:" << m_devices.keys();
    
    auto it = m_devices.find(deviceId);
    if (it == m_devices.end()) {
        qDebug() << "  Device not found, returning empty list";
        return QVariantList();
    }
    
    qDebug() << "  Device found, telemetry history size:" << it.value().telemetryHistory().size();
    QVariantList result = it.value().getTemperatureSeries();
    qDebug() << "  Returning" << result.size() << "temperature data points";
    if (result.size() > 0) {
        qDebug() << "  First point:" << result[0];
        qDebug() << "  Last point:" << result[result.size() - 1];
    }
    qDebug() << "=== getTemperatureData() END ===";
    return result;
}

QVariantList TelemetryViewModel::getPowerData(const QString& deviceId) const
{
    qDebug() << "  getPowerData() called for device:" << deviceId;
    auto it = m_devices.find(deviceId);
    if (it == m_devices.end()) {
        qDebug() << "  Device not found, returning empty list";
        return QVariantList();
    }
    QVariantList result = it.value().getPowerSeries();
    qDebug() << "  Returning" << result.size() << "power data points";
    if (result.size() > 0) {
        qDebug() << "  First point:" << result[0];
        qDebug() << "  Last point:" << result[result.size() - 1];
    }
    return result;
}

QVariantList TelemetryViewModel::getVoltageData(const QString& deviceId) const
{
    qDebug() << "  getVoltageData() called for device:" << deviceId;
    auto it = m_devices.find(deviceId);
    if (it == m_devices.end()) {
        qDebug() << "  Device not found, returning empty list";
        return QVariantList();
    }
    QVariantList result = it.value().getVoltageSeries();
    qDebug() << "  Returning" << result.size() << "voltage data points";
    if (result.size() > 0) {
        qDebug() << "  First point:" << result[0];
        qDebug() << "  Last point:" << result[result.size() - 1];
    }
    return result;
}

QVariantMap TelemetryViewModel::getLatestTelemetry(const QString& deviceId) const
{
    auto it = m_devices.find(deviceId);
    if (it == m_devices.end()) {
        return QVariantMap();
    }
    return it.value().getLatestValues();
}

QVariantList TelemetryViewModel::getErrorHistory(const QString& deviceId) const
{
    auto it = m_devices.find(deviceId);
    if (it == m_devices.end()) {
        return QVariantList();
    }
    return it.value().getErrorHistoryList();
}

QVariantList TelemetryViewModel::getErrorData(const QString& deviceId) const
{
    auto it = m_devices.find(deviceId);
    if (it == m_devices.end()) {
        return QVariantList();
    }
    return it.value().getErrorSeries();
}

QVariantList TelemetryViewModel::selectedDeviceTemperatureData() const
{
    qDebug() << "=== selectedDeviceTemperatureData() CALLED ===";
    qDebug() << "  selectedDevice:" << m_selectedDevice;
    qDebug() << "  dataUpdateCounter:" << m_dataUpdateCounter;
    QVariantList result = getTemperatureData(m_selectedDevice);
    qDebug() << "  Returning" << result.size() << "points";
    qDebug() << "=== selectedDeviceTemperatureData() END ===";
    return result;
}

QVariantList TelemetryViewModel::selectedDevicePowerData() const
{
    qDebug() << "=== selectedDevicePowerData() CALLED ===";
    qDebug() << "  selectedDevice:" << m_selectedDevice;
    qDebug() << "  dataUpdateCounter:" << m_dataUpdateCounter;
    QVariantList result = getPowerData(m_selectedDevice);
    qDebug() << "  Returning" << result.size() << "points";
    qDebug() << "=== selectedDevicePowerData() END ===";
    return result;
}

QVariantList TelemetryViewModel::selectedDeviceVoltageData() const
{
    qDebug() << "=== selectedDeviceVoltageData() CALLED ===";
    qDebug() << "  selectedDevice:" << m_selectedDevice;
    qDebug() << "  dataUpdateCounter:" << m_dataUpdateCounter;
    QVariantList result = getVoltageData(m_selectedDevice);
    qDebug() << "  Returning" << result.size() << "points";
    qDebug() << "=== selectedDeviceVoltageData() END ===";
    return result;
}

QVariantList TelemetryViewModel::getRecentErrors(int limit) const
{
    QVariantList allErrors;
    
    // Collect all errors from all devices
    for (const auto& deviceData : m_devices) {
        QVariantList deviceErrors = deviceData.getErrorHistoryList();
        for (const QVariant& error : deviceErrors) {
            QVariantMap errorMap = error.toMap();
            errorMap["device_id"] = deviceData.deviceId();
            allErrors.append(errorMap);
        }
    }
    
    // Sort by timestamp (most recent first)
    std::sort(allErrors.begin(), allErrors.end(), [](const QVariant& a, const QVariant& b) {
        return a.toMap()["timestamp"].toString() > b.toMap()["timestamp"].toString();
    });
    
    // Apply limit
    if (limit > 0 && allErrors.size() > limit) {
        allErrors = allErrors.mid(0, limit);
    }
    
    return allErrors;
}

void TelemetryViewModel::clearDeviceData(const QString& deviceId)
{
    auto it = m_devices.find(deviceId);
    if (it != m_devices.end()) {
        it.value().clearAll();
        emit telemetryDataUpdated(deviceId);
        emit errorDataUpdated(deviceId);
    }
}

void TelemetryViewModel::clearAllData()
{
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        it.value().clearAll();
        emit telemetryDataUpdated(it.key());
        emit errorDataUpdated(it.key());
    }
}

void TelemetryViewModel::setMaxHistoryPoints(int max)
{
    if (max > 0) {
        m_maxHistoryPoints = max;
        for (auto& deviceData : m_devices) {
            deviceData.setMaxTelemetryPoints(max);
        }
    }
}

void TelemetryViewModel::setMaxErrorEntries(int max)
{
    if (max > 0) {
        m_maxErrorEntries = max;
        for (auto& deviceData : m_devices) {
            deviceData.setMaxErrorEntries(max);
        }
    }
}

bool TelemetryViewModel::hasDevice(const QString& deviceId) const
{
    return m_devices.contains(deviceId);
}

void TelemetryViewModel::onTelemetryReceived(const QString& deviceId, double temperature, double power, double voltage)
{
    qDebug() << "=== TelemetryViewModel::onTelemetryReceived() CALLED ===";
    qDebug() << "  deviceId:" << deviceId;
    qDebug() << "  temperature:" << temperature;
    qDebug() << "  power:" << power;
    qDebug() << "  voltage:" << voltage;
    
    ensureDeviceExists(deviceId);
    
    models::TelemetryPoint point(deviceId, temperature, power, voltage, QDateTime::currentDateTime());
    auto it = m_devices.find(deviceId);
    if (it != m_devices.end()) {
        it.value().addTelemetryPoint(point);
        qDebug() << "  Telemetry point added. Total points for device:" << it.value().telemetryHistory().size();
    } else {
        qWarning() << "  ERROR: Device not found after ensureDeviceExists()!";
    }
    
    qDebug() << "  Emitting telemetryDataUpdated for device:" << deviceId;
    emit telemetryDataUpdated(deviceId);
    qDebug() << "  Signal emitted. Selected device:" << m_selectedDevice;
    
    // Auto-select first device if none selected
    if (m_selectedDevice.isEmpty()) {
        qDebug() << "  Auto-selecting device:" << deviceId;
        setSelectedDevice(deviceId);
    }
    
    // IMPORTANT: Increment data update counter to force QML bindings to re-evaluate
    // This fixes the issue where charts don't update because the binding
    // doesn't know to re-fetch data when telemetryDataUpdated is emitted
    m_dataUpdateCounter++;
    qDebug() << "  Incrementing dataUpdateCounter to:" << m_dataUpdateCounter;
    emit dataUpdateCounterChanged();
    qDebug() << "  dataUpdateCounterChanged signal emitted";
    
    qDebug() << "=== TelemetryViewModel::onTelemetryReceived() END ===";
}

void TelemetryViewModel::onErrorReceived(const QString& deviceId, const QString& errorText)
{
    ensureDeviceExists(deviceId);
    
    models::ErrorEntry entry(deviceId, errorText, QDateTime::currentDateTime());
    auto it = m_devices.find(deviceId);
    if (it != m_devices.end()) {
        it.value().addErrorEntry(entry);
    }
    
    emit errorDataUpdated(deviceId);
    emit newErrorReceived(deviceId, errorText);
    
    // Auto-select first device if none selected
    if (m_selectedDevice.isEmpty()) {
        setSelectedDevice(deviceId);
    }
}

void TelemetryViewModel::ensureDeviceExists(const QString& deviceId)
{
    qDebug() << "  ensureDeviceExists() called for device:" << deviceId;
    qDebug() << "  Current devices:" << m_devices.keys();
    
    if (!m_devices.contains(deviceId)) {
        qDebug() << "  Creating new device entry for:" << deviceId;
        auto it = m_devices.insert(deviceId, models::DeviceData(deviceId));
        it.value().setMaxTelemetryPoints(m_maxHistoryPoints);
        it.value().setMaxErrorEntries(m_maxErrorEntries);
        updateDeviceIds();
        qDebug() << "  Device created and deviceIds updated";
    } else {
        qDebug() << "  Device already exists, reusing existing entry";
    }
}

void TelemetryViewModel::updateDeviceIds()
{
    qDebug() << "  updateDeviceIds() called. Current device count:" << m_devices.size();
    emit deviceIdsChanged();
    qDebug() << "  deviceIdsChanged signal emitted";
}

void TelemetryViewModel::fetchHistoricalData()
{
    QString url = QString("http://%1:%2/api/telemetry/all")
                      .arg(m_serverAddress)
                      .arg(QString::number(m_serverPort));
    
    qDebug() << "Fetching historical data from:" << url;
    
    QNetworkRequest request(url);
    m_networkManager->get(request);
}

void TelemetryViewModel::onHistoricalDataReceived(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Failed to fetch historical data:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response received";
        reply->deleteLater();
        return;
    }
    
    QJsonObject rootObj = doc.object();
    if (!rootObj["success"].toBool()) {
        qWarning() << "API returned success: false";
        reply->deleteLater();
        return;
    }
    
    QJsonArray dataArray = rootObj["data"].toArray();
    qDebug() << "Received" << dataArray.size() << "historical telemetry points";
    
    for (const QJsonValue& value : dataArray) {
        QJsonObject telemetryObj = value.toObject();
        
        QString deviceId = telemetryObj["device_id"].toString();
        double temperature = telemetryObj["temperature"].toDouble();
        double power = telemetryObj["power"].toDouble();
        double voltage = telemetryObj["voltage"].toDouble();
        qint64 timestamp = telemetryObj["timestamp"].toVariant().toLongLong();
        
        ensureDeviceExists(deviceId);
        
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
        models::TelemetryPoint point(deviceId, temperature, power, voltage, dateTime);
        
        auto it = m_devices.find(deviceId);
        if (it != m_devices.end()) {
            it.value().addTelemetryPoint(point);
        }
    }
    
    // Emit signals to update UI
    emit deviceIdsChanged();
    emit dataUpdateCounterChanged();
    m_dataUpdateCounter++;
    
    // Auto-select first device if none selected
    if (m_selectedDevice.isEmpty() && !m_devices.isEmpty()) {
        setSelectedDevice(m_devices.keys().first());
    }
    
    // Emit telemetryDataUpdated for the selected device to update DeviceView
    if (!m_selectedDevice.isEmpty()) {
        emit telemetryDataUpdated(m_selectedDevice);
    }
    
    reply->deleteLater();
}

} // namespace viewmodels
} // namespace frontend
