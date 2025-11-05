import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCharts
import DataManager 1.0

ApplicationWindow {
    id: mainWindow
    width: 1200
    height: 900
    visible: true
    title: qsTr("TLM Analyzer")

    // Properties
    property string currentFolder: ""
    property double resistanceVoltage: 1.0
    
    // C++ DataManager instance
    DataManager {
        id: dataManager
        
        // Handle data changes from C++
        onDataChanged: {
            updateUIWithData()
        }
        
        // Set QML object reference for C++ to call QML functions
        Component.onCompleted: {
            dataManager.setQmlObject(mainWindow)
        }
    }
    
    // Update UI with data from DataManager
    function updateUIWithData() {
        // Update data point list
        updateDataPointList()
        
        // Update chart with new data
        chartData.updateChart(dataManager.getDataPointsForQml())
    }
    
    // C++ callable functions (called from C++ via callQmlFunction)
    function showMessage(message) {
        resultText.text = message
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
        completionAnimation.start()
    }
    
    // Update the data point list display
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
    
    // Menu
    header: MenuBar {
        Menu {
            title: qsTr("Help")
            
            MenuItem {
                text: qsTr("About")
                onTriggered: aboutDialog.open()
            }
        }
    }
    
    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        GroupBox {
            title: qsTr("Data Selection")
            Layout.fillWidth: true
            Layout.margins: 10
            
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
                }
            }
        }
        
        GroupBox {
            title: qsTr("Analysis Parameters")
            Layout.fillWidth: true
            Layout.margins: 10
            
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
                    // Call C++ analysis function
                    dataManager.performAnalysis()
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
                    
                    model: ListModel {
                        id: dataPointModel
                    }
                    
                    delegate: Rectangle {
                        width: dataPointList.width
                        height: 40
                        color: model.enabled ? "transparent" : "#f0f0f0"
                        
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
                            addPointDialog.open()
                        }
                    }
                    
                    Button {
                        id: removePointButton
                        text: qsTr("Remove Point")
                        enabled: dataPointList.currentIndex >= 0
                        onClicked: {
                            if (dataPointList.currentIndex >= 0) {
                                dataManager.setDataPointEnabled(dataPointList.currentIndex, false)
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
            Layout.margins: 10
            
            ChartData {
                id: chartData
                anchors.fill: parent
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
    
    // Dialogs
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Data Folder Containing CSV Files")
        
        onAccepted: {
            currentFolder = selectedFolder.toString().replace("file:///", "")
            folderPathField.text = currentFolder
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
                dataManager.addManualDataPoint(spacing, current, voltage)
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
}