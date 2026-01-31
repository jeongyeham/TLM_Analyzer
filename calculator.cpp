#include "include/calculator.h"
#include <cmath>

/**
 * @brief Perform linear regression analysis on TLM data points
 * @param dataPoints Vector of DataPoint objects to analyze
 * @param result Reference to TLMResult object to store calculated values
 * @return True if regression was successful, false otherwise
 * 
 * This method performs a linear regression analysis on the provided data points
 * to calculate TLM parameters including sheet resistance, contact resistance,
 * and specific contact resistivity. Only enabled data points are included in
 * the calculation.
 */
bool Calculator::linearRegression(const QVector<DataPoint> &dataPoints, Calculator::TLMResult &result)
{
    // Use default channel width of 100 μm for backward compatibility
    return linearRegression(dataPoints, result, 100.0);
}

/**
 * @brief Perform linear regression analysis on TLM data points with specified channel width
 * @param dataPoints Vector of DataPoint objects to analyze
 * @param result Reference to TLMResult object to store calculated values
 * @param channelWidth Width of the channel in μm
 * @return True if regression was successful, false otherwise
 * 
 * This method performs a linear regression analysis on the provided data points
 * to calculate TLM parameters including sheet resistance, contact resistance,
 * and specific contact resistivity. Only enabled data points are included in
 * the calculation.
 */
bool Calculator::linearRegression(const QVector<DataPoint> &dataPoints, Calculator::TLMResult &result, double channelWidth)
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
    result.channelWidth = channelWidth;
    result.sheetResistance = slope * channelWidth;  // Convert to Ω/sq
    result.contactResistance = intercept; // Contact resistance in Ω
    // Specific contact resistivity (legacy formula used by the app).
    // Keep same units as before but guard against division by zero.
    if (std::abs(result.sheetResistance) < 1e-15) {
        result.specificContactResistivity = 0.0;
    } else {
        result.specificContactResistivity = (result.contactResistance * result.contactResistance / result.sheetResistance) * 1e-2; // ρc in Ω·cm²
    }

    // Calculate and store R-squared for the fit
    result.rSquared = calculateRSquared(x, y, slope, intercept);

    return true;
}

/**
 * @brief Perform linear regression on x,y data pairs
 * @param x Vector of x values
 * @param y Vector of y values
 * @param slope Reference to store the calculated slope
 * @param intercept Reference to store the calculated y-intercept
 * @return True if regression was successful, false otherwise
 * 
 * This method performs a standard linear regression using the least squares method.
 * It uses numerically stable centering techniques to improve accuracy.
 */
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
    // Avoid narrowing conversion by casting n to double
    meanX /= static_cast<double>(n);
    meanY /= static_cast<double>(n);

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

/**
 * @brief Calculate the coefficient of determination (R-squared) for a linear fit
 * @param x Vector of x values
 * @param y Vector of y values
 * @param slope The slope of the fitted line
 * @param intercept The y-intercept of the fitted line
 * @return The R-squared value indicating goodness of fit (0.0 to 1.0)
 * 
 * This method calculates how well the linear model fits the data, with values
 * closer to 1.0 indicating a better fit.
 */
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
    meanY /= static_cast<double>(n);

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
    
    double r2 = 1.0 - (residualSumSquares / totalSumSquares);
    // Clamp to [0,1] for safety against numerical noise
    if (r2 < 0.0) r2 = 0.0;
    if (r2 > 1.0) r2 = 1.0;
    return r2;
}