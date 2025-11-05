import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCharts
import TLMAnalyzer 1.0
import DataManager 1.0

ApplicationWindow {
    id: mainWindow
    width: 1200
    height: 900
    visible: true
    title: qsTr("TLM Analyzer")

    // Properties to hold data
    property string currentFolder: ""
    property double resistanceVoltage: 1.0
    
    // Create an instance of DataManager
    DataManager {
        id: dataManager
        onDataChanged: {
            // Animate the data point list update
            dataPointList.opacity = 0
            dataPointListAnimation.start()
            
            // Update the chart with new data
            chartData.updateChart(dataManager.getDataPointsForQml())
        }
        
        Component.onCompleted: {
            // Set the QML object reference in C++
            dataManager.setQmlObject(mainWindow)
        }
    }
    
    // Animation for data point list
    SequentialAnimation {
        id: dataPointListAnimation
        NumberAnimation {
            target: dataPointList
            property: "opacity"
            to: 1
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }
    
    // Functions that can be called from C++
    function showMessage(message) {
        resultText.text = message
        // Add a fade-in animation for messages
        resultText.opacity = 0
        messageFadeInAnimation.start()
    }
    
    function updateProgressBar(value) {
        progressBar.value = value
        progressBar.visible = (value > 0 && value < 100)
    }
    
    function showAnalysisComplete() {
        analyzeButton.enabled = true
        progressBar.visible = false
        // Add a completion effect
        completionAnimation.start()
    }
    
    // Animations
    SequentialAnimation {
        id: messageFadeInAnimation
        NumberAnimation {
            target: resultText
            property: "opacity"
            to: 1
            duration: 500
            easing.type: Easing.InOutQuad
        }
    }
    
    SequentialAnimation {
        id: completionAnimation
        PropertyAnimation {
            target: analyzeButton
            property: "scale"
            to: 1.1
            duration: 200
        }
        PropertyAnimation {
            target: analyzeButton
            property: "scale"
            to: 1.0
            duration: 200
        }
    }
    
    header: MenuBar {
        Menu {
            title: qsTr("Help")
            
            MenuItem {
                text: qsTr("About")
                onTriggered: aboutDialog.open()
            }
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
    
    // Function to update the data point list
    function updateDataPointList() {
        // Clear the current list
        dataPointModel.clear()
        
        // Populate the list with data from DataManager
        for (var i = 0; i < dataManager.count; i++) {
            var point = dataManager.getDataPoint(i)
            dataPointModel.append({
                "index": i,
                "spacing": point.spacing,
                "resistance": point.resistance,
                "current": point.current,
                "enabled": point.enabled
            })
        }
    }
    
    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        padding: 10
        
        GroupBox {
            title: qsTr("Data Selection")
            Layout.fillWidth: true
            
            RowLayout {
                anchors.fill: parent
                
                TextField {
                    id: folderPathField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Select folder containing CSV files...")
                    text: currentFolder
                    selectByMouse: true
                }
                
                Button {
                    id: browseButton
                    text: qsTr("Browse...")
                    onClicked: folderDialog.open()
                    
                    // Add hover effect
                    MouseArea {
                        anchors.fill: parent
                        onPressed: browseButton.scale = 0.95
                        onReleased: browseButton.scale = 1.0
                        hoverEnabled: true
                        onEntered: browseButton.scale = 1.05
                        onExited: browseButton.scale = 1.0
                    }
                }
            }
        }
        
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
                    id: voltageField
                    text: "1.0"
                    validator: DoubleValidator {
                        bottom: -1000000.0
                        top: 1000000.0
                    }
                    
                    onTextChanged: {
                        var val = parseFloat(text)
                        if (!isNaN(val)) {
                            mainWindow.resistanceVoltage = val
                        }
                    }
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                id: analyzeButton
                text: qsTr("Analyze Data")
                background: Rectangle {
                    color: "#2196F3"
                    border.color: "#2196F3"
                    radius: 4
                    
                    // Add hover effect
                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                        }
                    }
                }
                contentItem: Text {
                    text: analyzeButton.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    // Example of calling C++ function that will call back to QML
                    dataManager.performAsyncAnalysis()
                }
                
                // Hover effects
                MouseArea {
                    anchors.fill: parent
                    onPressed: analyzeButton.scale = 0.95
                    onReleased: analyzeButton.scale = 1.0
                    hoverEnabled: true
                    onEntered: {
                        analyzeButton.background.color = "#1976D2"
                        analyzeButton.scale = 1.05
                    }
                    onExited: {
                        analyzeButton.background.color = "#2196F3"
                        analyzeButton.scale = 1.0
                    }
                }
            }
            
            Button {
                id: exportButton
                text: qsTr("Export Plot")
                background: Rectangle {
                    color: "#4CAF50"
                    border.color: "#4CAF50"
                    radius: 4
                    
                    // Add hover effect
                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                        }
                    }
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
                }
                
                // Hover effects
                MouseArea {
                    anchors.fill: parent
                    onPressed: exportButton.scale = 0.95
                    onReleased: exportButton.scale = 1.0
                    hoverEnabled: true
                    onEntered: {
                        exportButton.background.color = "#388E3C"
                        exportButton.scale = 1.05
                    }
                    onExited: {
                        exportButton.background.color = "#4CAF50"
                        exportButton.scale = 1.0
                    }
                }
            }
            
            Item {
                Layout.fillWidth: true
            }
        }
        
        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            visible: false
            indeterminate: false
            from: 0
            to: 100
            
            // Add smooth value change animation
            Behavior on value {
                NumberAnimation {
                    duration: 300
                }
            }
            
            // Add fade in/out effect
            Behavior on opacity {
                NumberAnimation {
                    duration: 500
                }
            }
        }
        
        GroupBox {
            title: qsTr("Data Points Management")
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 150
            
            RowLayout {
                anchors.fill: parent
                
                ListView {
                    id: dataPointList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    opacity: 1
                    
                    model: ListModel {
                        id: dataPointModel
                    }
                    
                    delegate: Rectangle {
                        width: dataPointList.width
                        height: 40
                        color: model.enabled ? "transparent" : "#f0f0f0"
                        
                        // Add entrance animation for list items
                        scale: 0.8
                        opacity: 0
                        Behavior on scale {
                            NumberAnimation {
                                duration: 300
                                easing.type: Easing.OutBack
                            }
                        }
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 300
                            }
                        }
                        
                        Component.onCompleted: {
                            scale = 1
                            opacity = 1
                        }
                        
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Point %1: Spacing=%2 μm, Resistance=%3 Ω, Current=%4 A".arg(model.index+1).arg(model.spacing).arg(model.resistance).arg(model.current))
                            color: model.enabled ? "black" : "gray"
                        }
                        
                        Text {
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.enabled ? "" : qsTr("(Removed)")
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
                            // Show dialog to add a new data point
                            addPointDialog.open()
                        }
                    }
                    
                    Button {
                        id: removePointButton
                        text: qsTr("Remove Point")
                        enabled: dataPointList.currentIndex >= 0
                        onClicked: {
                            if (dataPointList.currentIndex >= 0) {
                                dataManager.setDataPointEnabledFromQml(dataPointList.currentIndex, false)
                                dataPointList.currentIndex = -1
                            }
                        }
                    }
                    
                    Button {
                        id: clearDisabledPointsButton
                        text: qsTr("Clear Removed Points")
                        onClicked: {
                            dataManager.clearDisabledDataPoints()
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
            
            ChartData {
                id: chartData
                anchors.fill: parent
            }
        }
        
        GroupBox {
            title: qsTr("Analysis Results")
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            
            ScrollView {
                anchors.fill: parent
                
                TextArea {
                    id: resultText
                    readOnly: true
                    text: qsTr("Analysis results will appear here")
                    wrapMode: TextArea.Wrap
                    opacity: 1
                    
                    // Add fade effect for text changes
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 300
                        }
                    }
                }
            }
        }
    }
    
    // Folder selection dialog
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Data Folder Containing CSV Files")
        
        onAccepted: {
            currentFolder = selectedFolder.toString().replace("file:///", "")
            folderPathField.text = currentFolder
            
            // TODO: Implement folder scanning functionality
        }
    }
    
    // Dialog for adding a new data point
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
            
            Label {
                text: qsTr("Voltage (V):")
            }
            
            TextField {
                id: voltageFieldDialog
                Layout.fillWidth: true
                text: voltageField.text
                validator: DoubleValidator {
                    bottom: -1000000.0
                    top: 1000000.0
                }
            }
        }
        
        onAccepted: {
            var spacing = parseFloat(spacingField.text)
            var current = parseFloat(currentField.text)
            var voltage = parseFloat(voltageFieldDialog.text)
            
            if (!isNaN(spacing) && !isNaN(current) && !isNaN(voltage)) {
                // Add the data point through DataManager
                dataManager.addManualDataPointFromQml(spacing, current, voltage)
            }
            
            // Clear the fields
            spacingField.text = ""
            currentField.text = ""
            voltageFieldDialog.text = voltageField.text
        }
        
        onOpened: {
            voltageFieldDialog.text = voltageField.text
        }
    }
}