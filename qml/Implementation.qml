import QtQuick

QtObject {
    id: implementation

    property double qml_channelWidth: 100.0

    // Properties that mirror the UI properties
    property string qml_currentFolder: ""
    property var qml_dataPoints: []
    property double qml_resistanceVoltage: 1.0

    // Keep only necessary signals used by MainUI.qml
    signal qml_onAnalysisComplete(string resultMessage)
    signal qml_onDataChanged
    signal qml_onProgressUpdated(int progress)

    function c_addManualDataPoint(spacing, current, voltage) {
        if (typeof DataManager !== 'undefined')
            DataManager.c_addManualDataPoint(spacing, current, voltage);
    }
    function c_cancelLoad() {
        if (typeof DataManager !== 'undefined')
            DataManager.c_cancelLoad();
    }
    function c_clearDisabledDataPoints() {
        if (typeof DataManager !== 'undefined')
            DataManager.c_clearDisabledDataPoints();
    }

    // Minimal wrappers used by MainUI.qml
    function c_loadDataFromFolder() {
        if (typeof DataManager !== 'undefined')
            DataManager.c_loadDataFromFolder(qml_currentFolder);
    }
    function c_performAnalysis() {
        if (typeof DataManager !== 'undefined')
            DataManager.c_performAnalysis(qml_channelWidth);
    }
    function c_setDataPointEnabled(index, enabled) {
        if (typeof DataManager !== 'undefined')
            DataManager.c_setDataPointEnabled(index, enabled);
    }

    // Chart logic moved from ChartData.qml
    function calculateChartData(points, showLinearFit) {
        var scatterPoints = [];
        var fitPoints = [];
        var axisXRange = {
            min: 0,
            max: 1
        };
        var axisYRange = {
            min: 0,
            max: 1
        };

        var xs = [];
        var ys = [];
        for (var i = 0; i < points.length; ++i) {
            var p = points[i];
            if (!p.enabled)
                continue;
            scatterPoints.push({
                x: p.spacing,
                y: p.resistance
            });
            xs.push(p.spacing);
            ys.push(p.resistance);
        }

        if (xs.length > 0) {
            var minX = Math.min.apply(null, xs);
            var maxX = Math.max.apply(null, xs);
            var minY = Math.min.apply(null, ys);
            var maxY = Math.max.apply(null, ys);

            var xPadding = (maxX - minX) * 0.1;
            var yPadding = (maxY - minY) * 0.1;
            if (xPadding === 0)
                xPadding = 1;
            if (yPadding === 0)
                yPadding = 1;

            axisXRange.min = minX - xPadding;
            axisXRange.max = maxX + xPadding;
            axisYRange.min = Math.max(0, minY - yPadding);
            axisYRange.max = maxY + yPadding;

            if (showLinearFit && points.length > 1) {
                var sx = 0, sy = 0, sxy = 0, sxx = 0, n = 0;
                for (var j = 0; j < points.length; ++j) {
                    if (!points[j].enabled)
                        continue;
                    var xx = points[j].spacing, yy = points[j].resistance;
                    sx += xx;
                    sy += yy;
                    sxy += xx * yy;
                    sxx += xx * xx;
                    n++;
                }
                if (n > 1) {
                    var slope = (n * sxy - sx * sy) / (n * sxx - sx * sx);
                    var intercept = (sy - slope * sx) / n;
                    var x1 = axisXRange.min;
                    var x2 = axisXRange.max;
                    var y1 = slope * x1 + intercept;
                    var y2 = slope * x2 + intercept;
                    fitPoints.push({
                        x: x1,
                        y: y1
                    });
                    fitPoints.push({
                        x: x2,
                        y: y2
                    });
                }
            }
        }

        return {
            scatter: scatterPoints,
            fit: fitPoints,
            axisX: axisXRange,
            axisY: axisYRange
        };
    }

    // Helper to convert model to JS array for charting
    function modelToArray() {
        var arr = [];
        if (typeof DataManager === 'undefined' || !DataManager.model)
            return arr;
        var mdl = DataManager.model;
        var n = typeof mdl.count === 'function' ? mdl.count() : (typeof mdl.rowCount === 'function' ? mdl.rowCount() : 0);
        for (var i = 0; i < n; ++i) {
            var item = mdl.get(i);
            arr.push({
                spacing: item.spacing,
                resistance: item.resistance,
                current: item.current,
                enabled: item.enabled
            });
        }
        return arr;
    }

    Component.onCompleted: {
        if (typeof DataManager !== 'undefined') {
            implementation.qml_resistanceVoltage = DataManager.getResistanceVoltage();
            implementation.qml_channelWidth = DataManager.getChannelWidth();
        }
    }

    // Listen to global DataManager context property (set in main.cpp)
    Connections {
        target: DataManager

        onAnalysisComplete: function (resultMessage) {
            implementation.qml_onAnalysisComplete(resultMessage);
        }
        onDataChanged: function () {
            implementation.qml_onDataChanged();
        }
        onProgressUpdated: function (progress) {
            implementation.qml_onProgressUpdated(progress);
        }
    }
}