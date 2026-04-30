import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#121212"
    
    property var viewModel: null
    property string deviceId: ""
    property var temperatureData: []
    property var powerData: []
    property var voltageData: []
    property var errorData: []
    property var errorHistory: []
    
    signal deviceSelected(string deviceId)
    
    onDeviceIdChanged: {
        console.log("=== DeviceView::onDeviceIdChanged() ===");
        console.log("  deviceId:", deviceId);
        console.log("=== DeviceView::onDeviceIdChanged() END ===");
    }
    
    onTemperatureDataChanged: {
        console.log("=== DeviceView::onTemperatureDataChanged() ===");
        console.log("  temperatureData.length:", temperatureData ? temperatureData.length : "null/undefined");
        if (temperatureData && temperatureData.length > 0) {
            console.log("  First point:", temperatureData[0]);
            console.log("  Last point:", temperatureData[temperatureData.length - 1]);
        }
        console.log("=== DeviceView::onTemperatureDataChanged() END ===");
    }
    
    onPowerDataChanged: {
        console.log("=== DeviceView::onPowerDataChanged() ===");
        console.log("  powerData.length:", powerData ? powerData.length : "null/undefined");
        if (powerData && powerData.length > 0) {
            console.log("  First point:", powerData[0]);
            console.log("  Last point:", powerData[powerData.length - 1]);
        }
        console.log("=== DeviceView::onPowerDataChanged() END ===");
    }
    
    onVoltageDataChanged: {
        console.log("=== DeviceView::onVoltageDataChanged() ===");
        console.log("  voltageData.length:", voltageData ? voltageData.length : "null/undefined");
        if (voltageData && voltageData.length > 0) {
            console.log("  First point:", voltageData[0]);
            console.log("  Last point:", voltageData[voltageData.length - 1]);
        }
        console.log("=== DeviceView::onVoltageDataChanged() END ===");
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 5
        
        // Device header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "#1e1e1e"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                
                Text {
                    text: "Device:"
                    color: "#888888"
                    font.pixelSize: 14
                }
                
                Text {
                    text: root.deviceId || "No device selected"
                    color: "#ffffff"
                    font.pixelSize: 16
                    font.bold: true
                }
                
                Item {
                    Layout.fillWidth: true
                }
                
            }
        }
        
        // Latest values
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#1a1a1a"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 20
                
                // Temperature
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    
                    Text {
                        text: "Temperature"
                        color: "#888888"
                        font.pixelSize: 11
                    }
                    
                    Text {
                        text: {
                            if (root.temperatureData.length > 0) {
                                return root.temperatureData[root.temperatureData.length - 1].value.toFixed(2) + " °C";
                            }
                            return "-- °C";
                        }
                        color: "#3498db"
                        font.pixelSize: 20
                        font.bold: true
                    }
                }
                
                // Power
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    
                    Text {
                        text: "Power"
                        color: "#888888"
                        font.pixelSize: 11
                    }
                    
                    Text {
                        text: {
                            if (root.powerData.length > 0) {
                                return root.powerData[root.powerData.length - 1].value.toFixed(2) + " W";
                            }
                            return "-- W";
                        }
                        color: "#e67e22"
                        font.pixelSize: 20
                        font.bold: true
                    }
                }
                
                // Voltage
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    
                    Text {
                        text: "Voltage"
                        color: "#888888"
                        font.pixelSize: 11
                    }
                    
                    Text {
                        text: {
                            if (root.voltageData.length > 0) {
                                return root.voltageData[root.voltageData.length - 1].value.toFixed(2) + " V";
                            }
                            return "-- V";
                        }
                        color: "#9b59b6"
                        font.pixelSize: 20
                        font.bold: true
                    }
                }
            }
        }
        
        // Charts area
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 5
            columns: 2
            rows: 3
            rowSpacing: 5
            columnSpacing: 5

            // Temperature chart
            TelemetryChartView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Temperature"
                valueLabel: "Temperature (°C)"
                axisYTitle: "Temperature (°C)"
                lineColor: "#3498db"
                minY: 0
                maxY: 100
                dataSeries: root.temperatureData
            }

            // Power chart
            TelemetryChartView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Power"
                valueLabel: "Power (W)"
                axisYTitle: "Power (W)"
                lineColor: "#e67e22"
                minY: 0
                maxY: 500
                dataSeries: root.powerData
            }

            // Voltage chart - spans 2 rows for larger height
            TelemetryChartView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rowSpan: 2
                title: "Voltage"
                valueLabel: "Voltage (V)"
                axisYTitle: "Voltage (V)"
                lineColor: "#9b59b6"
                minY: 0
                maxY: 30
                dataSeries: root.voltageData
            }

            // Error view - spans 2 rows for larger height
            ErrorView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rowSpan: 2
                errorData: root.errorData
                errorHistory: root.errorHistory
            }
        }
    }
}
