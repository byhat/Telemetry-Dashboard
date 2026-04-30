import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#1e1e1e"
    
    property alias title: chartTitle.text
    property var dataSeries: []
    property string valueLabel: "Value"
    property string axisXTitle: "Time"
    property string axisYTitle: "Value"
    property color lineColor: "#3498db"
    property real minY: 0
    property real maxY: 100

    // Chart title
    Text {
        id: chartTitle
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        color: "#ffffff"
        font.pixelSize: 14
        font.bold: true
        z: 1
    }

    // Canvas for drawing the chart
    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.topMargin: 30
        anchors.leftMargin: 50
        anchors.rightMargin: 20
        anchors.bottomMargin: 40
        z: 0

        property real dataMinX: 0
        property real dataMaxX: 0
        property real dataMinY: 0
        property real dataMaxY: 0
        property var chartData: []

        onPaint: {
            console.log("=== Canvas::onPaint() ===");
            console.log("  title:", root.title);
            console.log("  canvas.width:", canvas.width, "canvas.height:", canvas.height);
            console.log("  dataMinX:", dataMinX, "dataMaxX:", dataMaxX);
            console.log("  dataMinY:", dataMinY, "dataMaxY:", dataMaxY);
            console.log("  chartData.length:", chartData ? chartData.length : "null/undefined");
            
            var ctx = getContext("2d");
            var width = canvas.width;
            var height = canvas.height;

            // Clear canvas
            ctx.clearRect(0, 0, width, height);

            // Draw grid
            ctx.strokeStyle = "#333333";
            ctx.lineWidth = 1;

            // Horizontal grid lines
            for (var i = 0; i <= 5; i++) {
                var y = (height / 5) * i;
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(width, y);
                ctx.stroke();
            }

            // Vertical grid lines
            for (var j = 0; j <= 5; j++) {
                var x = (width / 5) * j;
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, height);
                ctx.stroke();
            }

            // Draw axis labels
            ctx.fillStyle = "#ffffff";
            ctx.font = "10px sans-serif";
            ctx.textAlign = "right";

            // Y axis labels
            for (var k = 0; k <= 5; k++) {
                var yVal = dataMaxY - ((dataMaxY - dataMinY) / 5) * k;
                var yPos = (height / 5) * k;
                ctx.fillText(yVal.toFixed(1), -5, yPos + 3);
            }

            // Draw data line
            if (chartData && chartData.length > 0) {
                console.log("  Drawing data line with", chartData.length, "points");
                console.log("  First chart point:", chartData[0]);
                console.log("  Last chart point:", chartData[chartData.length - 1]);
                
                ctx.strokeStyle = root.lineColor;
                ctx.lineWidth = 2;
                ctx.beginPath();

                for (var m = 0; m < chartData.length; m++) {
                    var x = ((chartData[m].x - dataMinX) / (dataMaxX - dataMinX)) * width;
                    var y = height - ((chartData[m].y - dataMinY) / (dataMaxY - dataMinY)) * height;

                    if (m === 0) {
                        ctx.moveTo(x, y);
                    } else {
                        ctx.lineTo(x, y);
                    }
                }
                ctx.stroke();
                console.log("  Data line drawn");
            } else {
                console.log("  No chart data to draw");
            }
            console.log("=== Canvas::onPaint() END ===");
        }
    }

    // Y axis title
    Text {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10
        rotation: -90
        text: root.axisYTitle
        color: "#ffffff"
        font.pixelSize: 11
        z: 1
    }

    // X axis title
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 10
        text: root.axisXTitle
        color: "#ffffff"
        font.pixelSize: 11
        z: 1
    }

    function updateData(newData) {
        console.log("=== TelemetryChartView::updateData() ===");
        console.log("  title:", root.title);
        console.log("  newData:", newData);
        console.log("  newData type:", typeof newData);
        console.log("  newData.length:", newData ? newData.length : "null/undefined");
        
        if (newData && newData.length > 0) {
            console.log("  Processing", newData.length, "data points");
            console.log("  First data point:", newData[0]);
            console.log("  Last data point:", newData[newData.length - 1]);
            
            var minX = newData[0].timestamp;
            var maxX = newData[newData.length - 1].timestamp;

            var dataMin = Infinity;
            var dataMax = -Infinity;
            for (var j = 0; j < newData.length; j++) {
                if (newData[j].value < dataMin) dataMin = newData[j].value;
                if (newData[j].value > dataMax) dataMax = newData[j].value;
            }
            
            console.log("  dataMin:", dataMin, "dataMax:", dataMax);

            if (dataMin !== dataMax) {
                var padding = (dataMax - dataMin) * 0.1;
                canvas.dataMinY = dataMin - padding;
                canvas.dataMaxY = dataMax + padding;
            } else {
                canvas.dataMinY = dataMin - 1;
                canvas.dataMaxY = dataMax + 1;
            }

            if (minX !== maxX) {
                canvas.dataMinX = minX;
                canvas.dataMaxX = maxX;
            } else {
                canvas.dataMinX = minX - 1000;
                canvas.dataMaxX = maxX + 1000;
            }
            
            console.log("  X range:", canvas.dataMinX, "to", canvas.dataMaxX);
            console.log("  Y range:", canvas.dataMinY, "to", canvas.dataMaxY);

            // Convert data for canvas
            canvas.chartData = [];
            for (var i = 0; i < newData.length; i++) {
                canvas.chartData.push({
                    x: newData[i].timestamp,
                    y: newData[i].value
                });
            }
            
            console.log("  Converted to chartData, length:", canvas.chartData.length);
            console.log("  First chart point:", canvas.chartData[0]);
            console.log("  Last chart point:", canvas.chartData[canvas.chartData.length - 1]);

            canvas.requestPaint();
            console.log("  requestPaint() called");
        } else {
            console.log("  No data to process (newData is null, empty, or has length 0)");
        }
        console.log("=== TelemetryChartView::updateData() END ===");
    }

    Component.onCompleted: {
        console.log("=== TelemetryChartView::Component.onCompleted() ===");
        console.log("  title:", root.title);
        console.log("  dataSeries:", dataSeries);
        console.log("  dataSeries.length:", dataSeries ? dataSeries.length : "null/undefined");
        if (dataSeries && dataSeries.length > 0) {
            console.log("  Calling updateData from onCompleted");
            updateData(dataSeries);
        } else {
            console.log("  No data on component completion");
        }
        console.log("=== TelemetryChartView::Component.onCompleted() END ===");
    }

    onDataSeriesChanged: {
        console.log("=== TelemetryChartView::onDataSeriesChanged() ===");
        console.log("  title:", root.title);
        console.log("  dataSeries:", dataSeries);
        console.log("  dataSeries.length:", dataSeries ? dataSeries.length : "null/undefined");
        if (dataSeries && dataSeries.length > 0) {
            console.log("  Calling updateData from onDataSeriesChanged");
            updateData(dataSeries);
        } else {
            console.log("  No data on dataSeries change");
        }
        console.log("=== TelemetryChartView::onDataSeriesChanged() END ===");
    }

    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()
}
