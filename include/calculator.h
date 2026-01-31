#ifndef TLMANALYZER_CALCULATOR_H
#define TLMANALYZER_CALCULATOR_H

#include <QVector>
#include "datapoint.h"

/**
 * @brief Provides mathematical calculations for TLM analysis
 * 
 * This static class contains methods for performing linear regression
 * and other calculations needed for TLM (Transmission Line Model) analysis.
 */
class Calculator {
public:
    /**
     * @brief Results structure for TLM analysis
     */
    struct TLMResult {
        double slope;                      ///< Slope of the linear fit
        double intercept;                  ///< Y-intercept of the linear fit
        double rSquared;                   ///< R-squared value (coefficient of determination)
        double sheetResistance;            ///< Sheet resistance in Ω/sq
        double contactResistance;          ///< Contact resistance in Ω
        double specificContactResistivity; ///< Specific contact resistivity in Ω·cm²
        double channelWidth;               ///< Channel width in μm
        
        /**
         * @brief Default constructor
         * Initializes all values to zero
         */
        TLMResult() : slope(0.0), intercept(0.0), rSquared(0.0), sheetResistance(0.0), 
                     contactResistance(0.0), specificContactResistivity(0.0), channelWidth(100.0) {}
    };
    
    /**
     * @brief Perform linear regression on DataPoint vector
     * @param dataPoints Vector of data points to analyze
     * @param result Reference to TLMResult structure to store results
     * @return True if successful, false otherwise
     */
    static bool linearRegression(const QVector<DataPoint> &dataPoints, TLMResult &result);
    
    /**
     * @brief Perform linear regression on DataPoint vector with specified channel width
     * @param dataPoints Vector of data points to analyze
     * @param result Reference to TLMResult structure to store results
     * @param channelWidth Width of the channel in μm
     * @return True if successful, false otherwise
     */
    static bool linearRegression(const QVector<DataPoint> &dataPoints, TLMResult &result, double channelWidth);
    
    /**
     * @brief Perform linear regression on x,y value pairs
     * @param x Vector of x values
     * @param y Vector of y values
     * @param slope Reference to store calculated slope
     * @param intercept Reference to store calculated intercept
     * @return True if successful, false otherwise
     */
    static bool linearRegression(const QVector<double> &x, const QVector<double> &y, 
                                double &slope, double &intercept);
    
    /**
     * @brief Calculate R-squared value for a linear fit
     * @param x Vector of x values
     * @param y Vector of y values
     * @param slope Slope of the linear fit
     * @param intercept Y-intercept of the linear fit
     * @return R-squared value
     */
    static double calculateRSquared(const QVector<double> &x, const QVector<double> &y,
                                  double slope, double intercept);
    
private:
    /**
     * @brief Private constructor to prevent instantiation
     * This is a static utility class that should not be instantiated
     */
    Calculator() = default;
};

#endif // TLMANALYZER_CALCULATOR_H