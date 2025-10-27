#include "include/calculator.h"
#include <cmath>

bool Calculator::linearRegression(const QVector<DataPoint> &dataPoints, Calculator::TLMResult &result)
{
    if (dataPoints.size() < 2) {
        return false;
    }
    
    // Filter enabled data points
    QVector<double> x, y;
    for (const DataPoint &point : dataPoints) {
        if (point.enabled) {
            x.append(point.spacing);
            y.append(point.resistance);
        }
    }
    
    if (x.size() < 2) {
        return false;
    }
    
    // Perform linear regression
    double slope, intercept;
    if (!linearRegression(x, y, slope, intercept)) {
        return false;
    }
    
    result.slope = slope;
    result.intercept = intercept;
    result.sheetResistance = slope * 100.0;  // Convert to Ω/sq
    result.contactResistance = intercept / 20.0; // Contact resistance in Ω·mm
    result.specificContactResistivity = (result.contactResistance * result.contactResistance / result.sheetResistance) * 1e-2; // ρc in Ω·cm²
    
    return true;
}

bool Calculator::linearRegression(const QVector<double> &x, const QVector<double> &y,
                                 double &slope, double &intercept)
{
    if (x.size() != y.size() || x.size() < 2) {
        return false;
    }

    qsizetype n = x.size();
    
    // Calculate means
    double meanX = 0.0, meanY = 0.0;
    for (qsizetype i = 0; i < n; ++i) {
        meanX += x[i];
        meanY += y[i];
    }
    meanX /= n;
    meanY /= n;

    // Calculate centered sums for better numerical stability
    double sumXY_centered = 0.0, sumX2_centered = 0.0;
    for (qsizetype i = 0; i < n; ++i) {
        double x_centered = x[i] - meanX;
        double y_centered = y[i] - meanY;
        sumXY_centered += x_centered * y_centered;
        sumX2_centered += x_centered * x_centered;
    }

    // Check for zero denominator
    if (std::abs(sumX2_centered) < 1e-15) {
        return false;
    }

    // Calculate slope and intercept using centered data
    slope = sumXY_centered / sumX2_centered;
    intercept = meanY - slope * meanX;

    return true;
}

// Added function to calculate R-squared value
double Calculator::calculateRSquared(const QVector<double> &x, const QVector<double> &y, 
                                   double slope, double intercept)
{
    if (x.size() != y.size() || x.size() < 2) {
        return 0.0;
    }
    
    qsizetype n = x.size();
    
    // Calculate mean of y values
    double meanY = 0.0;
    for (qsizetype i = 0; i < n; ++i) {
        meanY += y[i];
    }
    meanY /= n;
    
    // Calculate total sum of squares and residual sum of squares
    double totalSumSquares = 0.0;
    double residualSumSquares = 0.0;
    
    for (qsizetype i = 0; i < n; ++i) {
        double predictedY = slope * x[i] + intercept;
        totalSumSquares += (y[i] - meanY) * (y[i] - meanY);
        residualSumSquares += (y[i] - predictedY) * (y[i] - predictedY);
    }
    
    // Calculate R-squared
    if (std::abs(totalSumSquares) < 1e-15) {
        return 1.0; // Perfect fit when all y values are the same
    }
    
    return 1.0 - (residualSumSquares / totalSumSquares);
}
