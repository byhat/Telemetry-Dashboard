#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>

// Qt Quick Charts (Qt 6.6+) is QML-only, no C++ initialization needed

#include "frontend/viewmodels/TelemetryViewModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("Telemetry Dashboard");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CrowRow");
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Telemetry Dashboard - Real-time telemetry visualization");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption serverUrlOption(QStringList() << "s" << "server",
                                       "WebSocket server URL",
                                       "url",
                                       "ws://localhost:18080/ws");
    parser.addOption(serverUrlOption);
    
    QCommandLineOption autoConnectOption(QStringList() << "a" << "auto-connect",
                                         "Automatically connect to server on startup");
    parser.addOption(autoConnectOption);
    
    parser.process(app);
    
    QString serverUrl = parser.value(serverUrlOption);
    bool autoConnect = parser.isSet(autoConnectOption);
    
    qDebug() << "Starting Telemetry Dashboard";
    qDebug() << "Server URL:" << serverUrl;
    qDebug() << "Auto-connect:" << (autoConnect ? "Yes" : "No");
    
    qDebug() << "[DEBUG] About to register QML types...";
    // Register QML types
    qmlRegisterType<frontend::viewmodels::TelemetryViewModel>("Telemetry", 1, 0, "TelemetryViewModel");
    qDebug() << "[DEBUG] QML types registered successfully";
    
    qDebug() << "[DEBUG] About to create ViewModel...";
    // Create ViewModel
    frontend::viewmodels::TelemetryViewModel* viewModel = new frontend::viewmodels::TelemetryViewModel(&app);
    qDebug() << "[DEBUG] ViewModel created successfully";
    viewModel->setServerUrl(serverUrl);
    qDebug() << "[DEBUG] Server URL set to:" << serverUrl;
    
    qDebug() << "[DEBUG] About to create QML engine...";
    // Create QML engine
    QQmlApplicationEngine engine;
    qDebug() << "[DEBUG] QML engine created successfully";
    
    qDebug() << "[DEBUG] About to expose ViewModel to QML...";
    // Expose ViewModel to QML
    engine.rootContext()->setContextProperty("viewModel", viewModel);
    qDebug() << "[DEBUG] ViewModel exposed to QML successfully";
    
    // Load QML
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    qDebug() << "[DEBUG] About to load QML from:" << url;
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        qDebug() << "[DEBUG] QML object created, obj:" << obj << "objUrl:" << objUrl;
        if (!obj && url == objUrl) {
            qCritical() << "[DEBUG] Failed to create QML object, exiting...";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    qDebug() << "[DEBUG] About to call engine.load(url)...";
    engine.load(url);
    qDebug() << "[DEBUG] engine.load(url) returned";
    
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
        return -1;
    }
    
    // Auto-connect if requested
    if (autoConnect) {
        qDebug() << "Auto-connecting to server...";
        viewModel->connectToServer();
    }
    
    return app.exec();
}
