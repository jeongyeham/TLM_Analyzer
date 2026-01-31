import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "./Implementation.qml" as Impl

ApplicationWindow {
    id: mainWindow
    width: 1200
    height: 900
    visible: true
    title: qsTr("TLM Analyzer")

    // Properties
    property string qml_currentFolder: ""
    property double qml_resistanceVoltage: 1.0
    property double qml_channelWidth: 100.0  // Default channel width in μm

    
    // Animations
    SequentialAnimation {
        id: messageFadeInAnimation
        NumberAnimation {
            target: resultText
            to: 1
            duration: 500
            easing.type: Easing.InOutQuad
        }
    }
    
    SequentialAnimation {
        id: completionAnimation
        PropertyAnimation {
            target: analyzeButton
            to: 1.1
            duration: 200
        }
        PropertyAnimation {
            target: analyzeButton
            to: 1.0
            duration: 200
        }
    }

    // Connections from the loaded Implementation object to the UI
    Connections {
        target: implLoader.item
        onQml_onAnalysisComplete: function(resultMessage) {
            resultText.text = resultMessage
            completionAnimation.start()
        }
        onQml_onDataChanged: function() {
            console.log("Data changed, updating UI")
            updateUIWithData()
        }
        onQml_onProgressUpdated: function(progress) {
            progressBar.value = progress
            progressBar.visible = (progress > 0 && progress < 100)
            cancelLoadButton.enabled = (progress > 0 && progress < 100)
            if (progress === 100) {
                console.log("Loading complete — running analysis")
                if (implLoader.item) implLoader.item.c_performAnalysis()
            }
        }
    }
    
    // Menu
    header: MenuBar {
        Menu {
            title: "Settings"
            
            MenuItem {
                text: "Preferences"
                onTriggered: settingsDialog.open()
            }
        }
        
        Menu {
            title: "Help"
            
            MenuItem {
                text: "About"
                onTriggered: aboutDialog.open()
            }
        }
    }
    
    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        GroupBox {
            title: "Data Selection"
            Layout.fillWidth: true
            Layout.margins: 10
            
            RowLayout {
                anchors.fill: parent
                
                TextField {
                    id: folderPathField
                    Layout.fillWidth: true
                    placeholderText: "Select folder containing CSV files..."
                    text: qml_currentFolder
                    selectByMouse: true
                }
                
                Button {
                    id: browseButton
                    text: "Browse..."
                    onClicked: {
                        folderDialog.open()
                    }
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 10
            
            Button {
                id: analyzeButton
                text: qsTr("Analyze Data")
                background: Rectangle {
                    color: "#2196F3"
                    border.color: "#2196F3"
                    radius: 4
                }
                contentItem: Text {
                    text: analyzeButton.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    implLoader.item.c_loadDataFromFolder()
                }
            }

            Button {
                id: cancelLoadButton
                text: qsTr("Cancel Load")
                enabled: false
                onClicked: {
   //TODO: Implement
                }
            }

            Button {
                id: exportButton
                text: qsTr("Export Plot")
                background: Rectangle {
                    color: "#4CAF50"
                    border.color: "#4CAF50"
                    radius: 4
                }
                contentItem: Text {
                    text: exportButton.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    // TODO: Implement export plot functionality
                    console.log("Export button clicked - functionality needs to be implemented")
                }
            }
            
            Item {
                Layout.fillWidth: true
            }
        }
        
        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            Layout.margins: 10
            visible: false
            indeterminate: false
            from: 0
            to: 100
        }
        
        GroupBox {
            title: qsTr("Data Points Management")
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 150
            Layout.margins: 10
            
            RowLayout {
                anchors.fill: parent
                
                ListView {
                    id: dataPointList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    highlight: Rectangle {
                        color: "#cce5ff"
                        radius: 2
                    }
                    highlightMoveDuration: 0
                    currentIndex: -1

                    model: DataManager.model

                    delegate: Rectangle {
                        width: dataPointList.width
                        height: 40
                        color: enabled ? (dataPointList.currentIndex === index ? "#cce5ff" : "transparent") : "#f0f0f0"

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Point %1: Spacing=%2 μm, Resistance=%3 Ω, Current=%4 A").arg(index+1).arg(spacing).arg(resistance).arg(current)
                            color: enabled ? "black" : "gray"
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: enabled ? "" : qsTr("(Removed)")
                            color: "red"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                dataPointList.currentIndex = index
                            }
                        }
                    }
                }
                
                ColumnLayout {
                    spacing: 5
                    
                    Button {
                        id: addPointButton
                        text: qsTr("Add Point")
                        onClicked: {
                            console.log("Adding manual data point")
                            addPointDialog.open()
                        }
                    }
                    
                    Button {
                        id: removePointButton
                        text: qsTr("Remove Point")
                        enabled: dataPointList.currentIndex >= 0
                        onClicked: {
                            if (dataPointList.currentIndex >= 0) {
                                implLoader.item.c_setDataPointEnabled(dataPointList.currentIndex, false)
                                dataPointList.currentIndex = -1
                                console.log("Point removed, updating UI")
                                updateUIWithData()
                            }
                        }
                    }
                    
                    Button {
                        id: clearDisabledPointsButton
                        text: qsTr("Clear Removed Points")
                        onClicked: {
                            console.log("Clearing disabled points")
                            implLoader.item.c_clearDisabledDataPoints()
                            updateUIWithData()
                        }
                    }
                    
                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
        
        GroupBox {
            title: qsTr("TLM Analysis Plot")
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 300
            Layout.margins: 10
            
            Canvas {
                id: plotCanvas
                anchors.fill: parent

                property var chartData: null
                property bool showLinearFit: true
                property string xAxisLabel: "Spacing (μm)"
                property string yAxisLabel: "Resistance (Ω)"
                property color dataPointColor: "#1976D2"
                property color fitLineColor: "#D32F2F"
                property color gridColor: "#e0e0e0"
                property color textColor: "black"
                property int padding: 50
                property real pointAnimationProgress: 0
                property real lineAnimationProgress: 0

                SequentialAnimation {
                    id: plotAnimation
                    running: false
                    NumberAnimation {
                        target: plotCanvas
                        property: "pointAnimationProgress"
                        from: 0
                        to: 1
                        duration: 500
                        easing.type: Easing.OutQuad
                    }
                    NumberAnimation {
                        target: plotCanvas
                        property: "lineAnimationProgress"
                        from: 0
                        to: 1
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }

                onPointAnimationProgressChanged: requestPaint()
                onLineAnimationProgressChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);

                    if (!chartData || !chartData.scatter || chartData.scatter.length === 0) {
                        ctx.fillStyle = textColor;
                        ctx.font = "16px sans-serif";
                        ctx.textAlign = "center";
                        ctx.fillText("No data to display", width / 2, height / 2);
                        return;
                    }

                    var plotWidth = width - padding * 2;
                    var plotHeight = height - padding * 2;

                    function toCanvasX(x) {
                        return padding + (x - chartData.axisX.min) / (chartData.axisX.max - chartData.axisX.min) * plotWidth;
                    }
                    function toCanvasY(y) {
                        return padding + plotHeight - (y - chartData.axisY.min) / (chartData.axisY.max - chartData.axisY.min) * plotHeight;
                    }

                    // Draw grid, axes and labels
                    ctx.beginPath();
                    ctx.strokeStyle = gridColor;
                    ctx.lineWidth = 1;
                    // Vertical grid lines
                    for (var i = 0; i <= 5; i++) {
                        var x = padding + (plotWidth / 5) * i;
                        ctx.moveTo(x, padding);
                        ctx.lineTo(x, height - padding);
                    }
                    // Horizontal grid lines
                    for (var j = 0; j <= 5; j++) {
                        var y = padding + (plotHeight / 5) * j;
                        ctx.moveTo(padding, y);
                        ctx.lineTo(width - padding, y);
                    }
                    ctx.stroke();

                    // Draw axis lines
                    ctx.beginPath();
                    ctx.strokeStyle = textColor;
                    ctx.moveTo(padding, padding);
                    ctx.lineTo(padding, height - padding);
                    ctx.lineTo(width - padding, height - padding);
                    ctx.stroke();

                    // Draw labels and titles
                    ctx.fillStyle = textColor;
                    ctx.textAlign = "center";
                    ctx.font = "12px sans-serif";
                    ctx.fillText(xAxisLabel, width / 2, height - 10);
                    ctx.save();
                    ctx.translate(15, height / 2);
                    ctx.rotate(-Math.PI / 2);
                    ctx.fillText(yAxisLabel, 0, 0);
                    ctx.restore();

                    // Draw axis values
                    ctx.textAlign = "right";
                    for (i = 0; i <= 5; i++) {
                        var val = chartData.axisY.min + (chartData.axisY.max - chartData.axisY.min) * (1 - i / 5);
                        ctx.fillText(val.toFixed(2), padding - 5, padding + (plotHeight / 5) * i + 4);
                    }
                    ctx.textAlign = "center";
                    for (i = 0; i <= 5; i++) {
                        val = chartData.axisX.min + (chartData.axisX.max - chartData.axisX.min) * (i / 5);
                        ctx.fillText(val.toFixed(2), padding + (plotWidth / 5) * i, height - padding + 15);
                    }

                    // Draw scatter points
                    ctx.fillStyle = dataPointColor;
                    for (i = 0; i < chartData.scatter.length; i++) {
                        var sx = toCanvasX(chartData.scatter[i].x);
                        var sy = toCanvasY(chartData.scatter[i].y);
                        ctx.beginPath();
                        ctx.arc(sx, sy, 4 * pointAnimationProgress, 0, 2 * Math.PI);
                        ctx.fill();
                    }

                    // Draw fit line
                    if (showLinearFit && chartData.fit && chartData.fit.length === 2) {
                        ctx.beginPath();
                        ctx.strokeStyle = fitLineColor;
                        ctx.lineWidth = 2;
                        var p1 = chartData.fit[0];
                        var p2 = chartData.fit[1];

                        var endX = p1.x + (p2.x - p1.x) * lineAnimationProgress;
                        var endY = p1.y + (p2.y - p1.y) * lineAnimationProgress;

                        ctx.moveTo(toCanvasX(p1.x), toCanvasY(p1.y));
                        ctx.lineTo(toCanvasX(endX), toCanvasY(endY));
                        ctx.stroke();
                    }
                }
            }
        }
        
        GroupBox {
            title: qsTr("Analysis Results")
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            Layout.margins: 10
            
            ScrollView {
                anchors.fill: parent
                
                TextArea {
                    id: resultText
                    readOnly: true
                    text: qsTr("Analysis results will appear here")
                    wrapMode: TextArea.Wrap
                    opacity: 1
                }
            }
        }
    }
    
    // Functions to update UI with data from C++
    function updateUIWithData() {
        plotAnimation.stop()
        plotCanvas.pointAnimationProgress = 0
        plotCanvas.lineAnimationProgress = 0

        var points = implLoader.item.modelToArray()
        plotCanvas.chartData = implLoader.item.calculateChartData(points, plotCanvas.showLinearFit)

        plotAnimation.start()
    }
    
    // Dialogs
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Data Folder Containing CSV Files")
        
        onAccepted: {
            qml_currentFolder = selectedFolder.toString().replace("file:///", "")
            implLoader.item.qml_currentFolder = qml_currentFolder
            folderPathField.text = qml_currentFolder
        }
    }
    
    Dialog {
        id: addPointDialog
        title: qsTr("Add Data Point")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        width: 300
        height: 250
        
        ColumnLayout {
            anchors.fill: parent
            
            Label {
                text: qsTr("Spacing (μm):")
            }
            
            TextField {
                id: spacingField
                Layout.fillWidth: true
                validator: DoubleValidator {
                    bottom: -1000000.0
                    top: 1000000.0
                }
            }
            
            Label {
                text: qsTr("Current (A):")
            }
            
            TextField {
                id: currentField
                Layout.fillWidth: true
                validator: DoubleValidator {
                    bottom: -1000000.0
                    top: 1000000.0
                }
            }
        }
        
        onAccepted: {
            var spacing = parseFloat(spacingField.text)
            var current = parseFloat(currentField.text)
            
            if (!isNaN(spacing) && !isNaN(current)) {
                // Add the data point through implementation
                // Use the voltage from settings
                implLoader.item.c_addManualDataPoint(spacing, current, implLoader.item.qml_resistanceVoltage)
            }
            
            // Clear the fields
            spacingField.text = ""
            currentField.text = ""
        }
    }
    
    // About Dialog
    Dialog {
        id: aboutDialog
        title: qsTr("About TLM Analyzer")
        standardButtons: Dialog.Ok
        modal: true
        width: 400
        height: 400
        
        ScrollView {
            anchors.fill: parent
            
            TextEdit {
                text: qsTr("<h2>TLM Analyzer</h2>" +
                          "<p><b>Version:</b> 2.0</p>" +
                          "<p><b>Description:</b> TLM Analyzer is a specialized tool for analyzing Transmission Line Model data from CSV files.</p>" +
                          "<p><b>Written by JeongYeham implementing Qt6.</b></p>" +
                          "<p><b>Special THANKS to Pudd1ng!!</b></p>" +
                          "<p><b>License:</b> MIT License</p>")
                readOnly: true
                textFormat: TextEdit.RichText
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }
    
    // Settings Dialog
    Dialog {
        id: settingsDialog
        title: qsTr("Preferences")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        width: 400
        height: 300
        
        onAccepted: {
            // Save the settings
            var resistanceVoltage = parseFloat(resistanceVoltageField.text)
            var channelWidth = parseFloat(channelWidthField.text)
            
            if (!isNaN(resistanceVoltage)) {
                qml_resistanceVoltage = resistanceVoltage
                implLoader.item.qml_resistanceVoltage = resistanceVoltage
                implLoader.item.c_setResistanceVoltage(resistanceVoltage)
            }
            
            if (!isNaN(channelWidth) && channelWidth > 0) {
                qml_channelWidth = channelWidth
                implLoader.item.qml_channelWidth = channelWidth
                implLoader.item.c_setChannelWidth(channelWidth)
            }
        }
        
        onOpened: {
            // Load current values when dialog opens
            resistanceVoltageField.text = qml_resistanceVoltage.toString()
            channelWidthField.text = qml_channelWidth.toString()
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 15
            
            GroupBox {
                title: qsTr("Analysis Parameters")
                Layout.fillWidth: true
                
                GridLayout {
                    columns: 2
                    rowSpacing: 10
                    columnSpacing: 10
                    
                    Label {
                        text: qsTr("Resistance Voltage (V):")
                    }
                    
                    TextField {
                        id: resistanceVoltageField
                        Layout.fillWidth: true
                        validator: DoubleValidator {
                            bottom: -1000000.0
                            top: 1000000.0
                        }
                    }
                    
                    Label {
                        text: qsTr("Channel Width (μm):")
                    }
                    
                    TextField {
                        id: channelWidthField
                        Layout.fillWidth: true
                        validator: DoubleValidator {
                            bottom: 0.001
                            top: 1000000.0
                        }
                    }
                }
            }
            
            Label {
                text: qsTr("Note: These settings will be saved and used as defaults for future sessions.")
                wrapMode: Text.Wrap
                font.italic: true
                Layout.fillWidth: true
            }
            
            Item {
                Layout.fillHeight: true
            }
        }
    }
    
    // Initialize with config values
    Component.onCompleted: {
        // Load initial values from config
        qml_resistanceVoltage = DataManager.getResistanceVoltage()
        qml_channelWidth = DataManager.getChannelWidth()
        // Initial chart update
        updateUIWithData()
    }
}