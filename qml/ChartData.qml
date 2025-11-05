import QtQuick
import QtQuick.Controls

Item {
    id: chartContainer
    
    // Properties for chart configuration
    property alias chartView: chartView
    property var dataPoints: []
    property bool showLinearFit: true
    property string xAxisLabel: "Spacing (μm)"
    property string yAxisLabel: "Resistance (Ω)"
    
    // Chart margins
    property int marginLeft: 60
    property int marginRight: 20
    property int marginTop: 20
    property int marginBottom: 50
    
    // Colors
    property color gridColor: "#e0e0e0"
    property color axisColor: "#333333"
    property color dataPointColor: "#2196F3"
    property color fitLineColor: "#f44336"
    property color dataPointLineColor: "#FF9800"
    
    Rectangle {
        anchors.fill: parent
        color: "white"
        border.color: "#cccccc"
        border.width: 1
        
        Canvas {
            id: chartCanvas
            anchors.fill: parent
            antialiasing: true
            
            onPaint: {
                var ctx = getContext("2d");
                var width = chartCanvas.width;
                var height = chartCanvas.height;
                
                // Clear canvas
                ctx.clearRect(0, 0, width, height);
                
                // Draw chart background
                ctx.fillStyle = "white";
                ctx.fillRect(0, 0, width, height);
                
                // Calculate chart area
                var chartWidth = width - marginLeft - marginRight;
                var chartHeight = height - marginTop - marginBottom;
                
                if (chartWidth <= 0 || chartHeight <= 0) return;
                
                // Draw grid and axes only if we have data
                if (dataPoints.length > 0) {
                    drawGrid(ctx, width, height, chartWidth, chartHeight);
                    drawAxes(ctx, width, height, chartWidth, chartHeight);
                    drawDataPoints(ctx, width, height, chartWidth, chartHeight);
                    if (showLinearFit) {
                        drawLinearFit(ctx, width, height, chartWidth, chartHeight);
                    }
                }
                
                // Draw axis labels
                drawAxisLabels(ctx, width, height);
            }
            
            function drawGrid(ctx, width, height, chartWidth, chartHeight) {
                ctx.strokeStyle = gridColor;
                ctx.lineWidth = 1;
                
                // Draw horizontal grid lines
                var ySteps = 5;
                for (var i = 0; i <= ySteps; i++) {
                    var y = marginTop + (i * chartHeight / ySteps);
                    ctx.beginPath();
                    ctx.moveTo(marginLeft, y);
                    ctx.lineTo(marginLeft + chartWidth, y);
                    ctx.stroke();
                }
                
                // Draw vertical grid lines
                var xSteps = 5;
                for (var j = 0; j <= xSteps; j++) {
                    var x = marginLeft + (j * chartWidth / xSteps);
                    ctx.beginPath();
                    ctx.moveTo(x, marginTop);
                    ctx.lineTo(x, marginTop + chartHeight);
                    ctx.stroke();
                }
            }
            
            function drawAxes(ctx, width, height, chartWidth, chartHeight) {
                ctx.strokeStyle = axisColor;
                ctx.lineWidth = 2;
                
                // X axis
                ctx.beginPath();
                ctx.moveTo(marginLeft, marginTop + chartHeight);
                ctx.lineTo(marginLeft + chartWidth, marginTop + chartHeight);
                ctx.stroke();
                
                // Y axis
                ctx.beginPath();
                ctx.moveTo(marginLeft, marginTop);
                ctx.lineTo(marginLeft, marginTop + chartHeight);
                ctx.stroke();
            }
            
            function drawAxisLabels(ctx, width, height) {
                ctx.fillStyle = axisColor;
                ctx.font = "12px Arial";
                ctx.textAlign = "center";
                
                // X axis label
                ctx.fillText(xAxisLabel, width / 2, height - 10);
                
                // Y axis label (rotated)
                ctx.save();
                ctx.translate(20, height / 2);
                ctx.rotate(-Math.PI / 2);
                ctx.textAlign = "center";
                ctx.fillText(yAxisLabel, 0, 0);
                ctx.restore();
            }
            
            function drawDataPoints(ctx, width, height, chartWidth, chartHeight) {
                if (dataPoints.length === 0) return;
                
                // Find min/max values for scaling
                var minX = Math.min.apply(Math, dataPoints.map(function(p) { return p.spacing; }));
                var maxX = Math.max.apply(Math, dataPoints.map(function(p) { return p.spacing; }));
                var minY = Math.min.apply(Math, dataPoints.map(function(p) { return p.resistance; }));
                var maxY = Math.max.apply(Math, dataPoints.map(function(p) { return p.resistance; }));
                
                // Add some padding
                var xPadding = (maxX - minX) * 0.05;
                var yPadding = (maxY - minY) * 0.05;
                if (xPadding === 0) xPadding = 1;
                if (yPadding === 0) yPadding = 1;
                
                minX -= xPadding;
                maxX += xPadding;
                minY -= yPadding;
                maxY += yPadding;
                
                // Draw data points
                ctx.fillStyle = dataPointColor;
                ctx.strokeStyle = dataPointLineColor;
                ctx.lineWidth = 2;
                
                for (var i = 0; i < dataPoints.length; i++) {
                    var point = dataPoints[i];
                    if (!point.enabled) continue;
                    
                    var x = marginLeft + ((point.spacing - minX) / (maxX - minX)) * chartWidth;
                    var y = marginTop + chartHeight - ((point.resistance - minY) / (maxY - minY)) * chartHeight;
                    
                    // Draw point
                    ctx.beginPath();
                    ctx.arc(x, y, 5, 0, 2 * Math.PI);
                    ctx.fill();
                    
                    // Draw connecting line between points
                    if (i > 0) {
                        var prevPoint = dataPoints[i-1];
                        if (prevPoint.enabled) {
                            var prevX = marginLeft + ((prevPoint.spacing - minX) / (maxX - minX)) * chartWidth;
                            var prevY = marginTop + chartHeight - ((prevPoint.resistance - minY) / (maxY - minY)) * chartHeight;
                            
                            ctx.beginPath();
                            ctx.moveTo(prevX, prevY);
                            ctx.lineTo(x, y);
                            ctx.stroke();
                        }
                    }
                }
            }
            
            function drawLinearFit(ctx, width, height, chartWidth, chartHeight) {
                if (dataPoints.length < 2) return;
                
                // Filter enabled points
                var enabledPoints = dataPoints.filter(function(p) { return p.enabled; });
                if (enabledPoints.length < 2) return;
                
                // Calculate linear regression
                var sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
                for (var i = 0; i < enabledPoints.length; i++) {
                    var point = enabledPoints[i];
                    sumX += point.spacing;
                    sumY += point.resistance;
                    sumXY += point.spacing * point.resistance;
                    sumXX += point.spacing * point.spacing;
                }
                
                var n = enabledPoints.length;
                var slope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
                var intercept = (sumY - slope * sumX) / n;
                
                // Find min/max x values for line endpoints
                var minX = Math.min.apply(Math, enabledPoints.map(function(p) { return p.spacing; }));
                var maxX = Math.max.apply(Math, enabledPoints.map(function(p) { return p.spacing; }));
                
                // Add padding
                var xPadding = (maxX - minX) * 0.05;
                minX -= xPadding;
                maxX += xPadding;
                
                // Find min/max y values for scaling
                var minY = Math.min.apply(Math, dataPoints.map(function(p) { return p.resistance; }));
                var maxY = Math.max.apply(Math, dataPoints.map(function(p) { return p.resistance; }));
                var yPadding = (maxY - minY) * 0.05;
                if (yPadding === 0) yPadding = 1;
                minY -= yPadding;
                maxY += yPadding;
                
                // Calculate line endpoints
                var y1 = slope * minX + intercept;
                var y2 = slope * maxX + intercept;
                
                // Scale to canvas coordinates
                var x1 = marginLeft + ((minX - (minX - xPadding)) / ((maxX + xPadding) - (minX - xPadding))) * chartWidth;
                var x2 = marginLeft + ((maxX - (minX - xPadding)) / ((maxX + xPadding) - (minX - xPadding))) * chartWidth;
                var y1Canvas = marginTop + chartHeight - ((y1 - minY) / (maxY - minY)) * chartHeight;
                var y2Canvas = marginTop + chartHeight - ((y2 - minY) / (maxY - minY)) * chartHeight;
                
                // Draw fit line
                ctx.strokeStyle = fitLineColor;
                ctx.lineWidth = 2;
                ctx.setLineDash([5, 3]); // Dashed line
                ctx.beginPath();
                ctx.moveTo(x1, y1Canvas);
                ctx.lineTo(x2, y2Canvas);
                ctx.stroke();
                ctx.setLineDash([]); // Reset to solid line
                
                // Draw equation text
                ctx.fillStyle = fitLineColor;
                ctx.font = "bold 12px Arial";
                ctx.textAlign = "left";
                var equation = "y = " + slope.toFixed(4) + "x + " + intercept.toFixed(4);
                ctx.fillText(equation, marginLeft + 10, marginTop + 20);
            }
        }
    }
    
    // Function to update chart with new data
    function updateChart(points) {
        dataPoints = points;
        chartCanvas.requestPaint();
    }
}