import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 720
    title: qsTr("Telemetry Dashboard")
    color: "#121212"
    
    function updateDeviceData(deviceView) {
        if (!viewModel) return;
        var deviceId = viewModel.selectedDevice;
        if (!deviceId) return;
        
        console.log("=== Main.qml::updateDeviceData() ===");
        console.log("  deviceId:", deviceId);
        
        var tempData = viewModel.getTemperatureData(deviceId);
        console.log("  tempData from ViewModel:", tempData);
        console.log("  tempData type:", typeof tempData);
        console.log("  tempData.length:", tempData ? tempData.length : "null/undefined");
        
        var powerData = viewModel.getPowerData(deviceId);
        console.log("  powerData from ViewModel:", powerData);
        console.log("  powerData.length:", powerData ? powerData.length : "null/undefined");
        
        var voltageData = viewModel.getVoltageData(deviceId);
        console.log("  voltageData from ViewModel:", voltageData);
        console.log("  voltageData.length:", voltageData ? voltageData.length : "null/undefined");
        
        deviceView.temperatureData = tempData;
        deviceView.powerData = powerData;
        deviceView.voltageData = voltageData;
        deviceView.errorData = viewModel.getErrorData(deviceId);
        deviceView.errorHistory = viewModel.getErrorHistory(deviceId);
        
        console.log("  After assignment, deviceView.temperatureData.length:", deviceView.temperatureData.length);
        console.log("=== Main.qml::updateDeviceData() END ===");
    }
    
    function updateErrorData(errorView) {
        console.log("=== Main.qml::updateErrorData() ===");
        console.log("  errorView:", errorView);
        console.log("  errorView.errorData:", errorView.errorData);
        console.log("  errorView.errorHistory:", errorView.errorHistory);
        
        if (!viewModel) return;
        var deviceId = viewModel.selectedDevice;
        if (!deviceId) return;
        
        console.log("  deviceId:", deviceId);
        var errorData = viewModel.getErrorData(deviceId);
        console.log("  errorData from ViewModel:", errorData);
        
        errorView.errorData = errorData;
        errorView.errorHistory = viewModel.getErrorHistory(deviceId);
        
        console.log("=== Main.qml::updateErrorData() END ===");
    }
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Sidebar
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 220
            color: "#1e1e1e"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // Header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "#2d2d2d"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 5
                        
                        Text {
                            text: "Telemetry"
                            color: "#ffffff"
                            font.pixelSize: 18
                            font.bold: true
                        }
                        
                        Text {
                            text: "Dashboard"
                            color: "#888888"
                            font.pixelSize: 12
                        }
                    }
                }
                
                // Connection controls
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    color: "#1a1a1a"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 8
                        
                        Text {
                            text: "Connection"
                            color: "#888888"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        
                        // Server Address Input
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            color: "#2d2d2d"
                            radius: 4
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 8
                                
                                Text {
                                    text: "IP:"
                                    color: "#888888"
                                    font.pixelSize: 11
                                }
                                
                                TextField {
                                    Layout.fillWidth: true
                                    text: viewModel ? viewModel.serverAddress : "localhost"
                                    color: "#ffffff"
                                    font.pixelSize: 11
                                    placeholderText: "localhost"
                                    placeholderTextColor: "#666666"
                                    background: Rectangle {
                                        color: "transparent"
                                    }
                                    onTextChanged: {
                                        if (viewModel) {
                                            viewModel.serverAddress = text;
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Port Input
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            color: "#2d2d2d"
                            radius: 4
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 8
                                
                                Text {
                                    text: "Port:"
                                    color: "#888888"
                                    font.pixelSize: 11
                                }
                                
                                TextField {
                                    Layout.fillWidth: true
                                    text: viewModel ? viewModel.serverPort : "18080"
                                    color: "#ffffff"
                                    font.pixelSize: 11
                                    placeholderText: "18080"
                                    placeholderTextColor: "#666666"
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    validator: IntValidator { bottom: 1; top: 65535; }
                                    background: Rectangle {
                                        color: "transparent"
                                    }
                                    onTextChanged: {
                                        if (viewModel) {
                                            var port = parseInt(text, 10);
                                            if (!isNaN(port)) {
                                                viewModel.serverPort = port;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Connection Error Display
                        Text {
                            Layout.fillWidth: true
                            text: viewModel ? viewModel.connectionError : ""
                            color: "#e74c3c"
                            font.pixelSize: 10
                            wrapMode: Text.Wrap
                            visible: viewModel && viewModel.connectionError !== ""
                        }
                        
                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 32
                            text: viewModel && viewModel.connected ? "Disconnect" : "Connect"
                            enabled: viewModel !== null
                            
                            background: Rectangle {
                                color: parent.enabled ? (viewModel && viewModel.connected ? "#e74c3c" : "#2ecc71") : "#444444"
                                radius: 4
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 12
                            }
                            
                            onClicked: {
                                console.log("=== CONNECT BUTTON CLICKED ===");
                                console.log("viewModel is null:", viewModel === null);
                                console.log("viewModel.connected:", viewModel ? viewModel.connected : "N/A");
                                console.log("viewModel.serverAddress:", viewModel ? viewModel.serverAddress : "N/A");
                                console.log("viewModel.serverPort:", viewModel ? viewModel.serverPort : "N/A");
                                
                                if (viewModel.connected) {
                                    console.log("Calling disconnectFromServer()");
                                    viewModel.disconnectFromServer();
                                } else {
                                    console.log("Clearing connection error");
                                    viewModel.connectionError = "";
                                    console.log("Calling connectToServer()");
                                    viewModel.connectToServer();
                                    console.log("connectToServer() returned");
                                }
                                console.log("=== BUTTON CLICK HANDLER END ===");
                            }
                        }
                        
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 20
                            color: "transparent"
                            
                            RowLayout {
                                anchors.fill: parent
                                spacing: 5
                                
                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: viewModel && viewModel.connected ? "#2ecc71" : "#e74c3c"
                                }
                                
                                Text {
                                    text: viewModel && viewModel.connected ? "Connected" : "Disconnected"
                                    color: viewModel && viewModel.connected ? "#2ecc71" : "#e74c3c"
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }
                
                // Device list header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    color: "#2d2d2d"
                    
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 15
                        text: "Devices"
                        color: "#ffffff"
                        font.pixelSize: 14
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                // Device list
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    
                    ListView {
                        id: deviceListView
                        model: viewModel ? viewModel.deviceIds : []
                        spacing: 2
                        
                        delegate: Rectangle {
                            width: deviceListView.width
                            height: 45
                            color: viewModel.selectedDevice === modelData ? "#3498db" : (mouseArea.containsMouse ? "#2a2a2a" : "#1a1a1a")
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 15
                                anchors.rightMargin: 15
                                spacing: 10
                                
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData
                                    color: viewModel.selectedDevice === modelData ? "#ffffff" : "#cccccc"
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                }
                            }
                            
                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    viewModel.selectedDevice = modelData;
                                    deviceView.deviceId = modelData;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Main content area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#121212"
            
            DeviceView {
                id: deviceView
                anchors.fill: parent
                anchors.margins: 10
                viewModel: viewModel
                Component.onCompleted: {
                    console.log("=== DeviceView::Component.onCompleted() ===");
                    console.log("  viewModel:", viewModel);
                    if (viewModel) {
                        console.log("  viewModel.selectedDevice:", viewModel.selectedDevice);
                        console.log("  Updating data from onCompleted");
                        updateDeviceData(deviceView);
                    }
                    console.log("=== DeviceView::Component.onCompleted() END ===");
                }
            }
        }
    }
    
    // Connections to update UI when data changes
    Connections {
        target: viewModel
        
        function onTelemetryDataUpdated(deviceId) {
            console.log("=== Main.qml::onTelemetryDataUpdated() ===");
            console.log("  deviceId:", deviceId);
            console.log("  selectedDevice:", viewModel.selectedDevice);
            
            if (viewModel.selectedDevice === deviceId) {
                console.log("  Updating device view for selected device");
                updateDeviceData(deviceView);
            }
            console.log("=== Main.qml::onTelemetryDataUpdated() END ===");
        }
        
        function onErrorDataUpdated(deviceId) {
            console.log("=== Main.qml::onErrorDataUpdated() ===");
            console.log("  deviceId:", deviceId);
            console.log("  selectedDevice:", viewModel.selectedDevice);
            
            if (viewModel.selectedDevice === deviceId) {
                console.log("  Updating error history for selected device");
                updateDeviceData(deviceView);
            }
            console.log("=== Main.qml::onErrorDataUpdated() END ===");
        }
        
        function onSelectedDeviceChanged() {
            console.log("=== Main.qml::onSelectedDeviceChanged() ===");
            console.log("  selectedDevice:", viewModel.selectedDevice);
            console.log("  viewModel.connected:", viewModel.connected);
            console.log("  deviceView object:", deviceView);
            console.log("  deviceView.deviceId:", deviceView.deviceId);
            console.log("  errorView object:", errorView);
            console.log("  errorView.errorData:", errorView.errorData);
            console.log("  errorView.errorHistory:", errorView.errorHistory);
            
            if (viewModel.selectedDevice) {
                updateDeviceData(deviceView);
                updateErrorData(errorView);

            }
            
            console.log("=== Main.qml::onSelectedDeviceChanged() END ===");
        }
    }
}

