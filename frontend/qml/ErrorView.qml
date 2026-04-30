import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#1e1e1e"

    property var errorData: []
    property var errorHistory: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        

        // Error list header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "#2d2d2d"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10

                Text {
                    Layout.preferredWidth: parent.width * 0.3
                    text: "Timestamp"
                    color: "#888888"
                    font.pixelSize: 12
                    font.bold: true
                }

                Text {
                    Layout.fillWidth: true
                    text: "Error Message"
                    color: "#888888"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }

        // Error list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: errorListView
                model: errorHistory
                spacing: 2

                delegate: Rectangle {
                    width: errorListView.width
                    height: 40
                    color: index % 2 === 0 ? "#252525" : "#2a2a2a"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 10

                        Text {
                            Layout.preferredWidth: parent.width * 0.3
                            text: modelData.timestamp || ""
                            color: "#aaaaaa"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: modelData.errorText || ""
                            color: "#e74c3c"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (errorHistory && errorHistory.length > 0) {
            errorListView.model = errorHistory;
        }
    }

    onErrorHistoryChanged: {
        errorListView.model = errorHistory;
    }
}
