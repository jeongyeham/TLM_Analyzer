#ifndef TLMANALYZER_CALCULATOR_H
#define TLMANALYZER_CALCULATOR_H

#include <QVector>
#include "datapoint.h"

class Calculator {
public:
    struct TLMResult {
        double slope;
        double intercept;
        double rSquared;            // R² value
        double sheetResistance;     // Rsh in Ω/sq
        double contactResistance;   // Rc in Ω·mm
        double specificContactResistivity; // ρc in Ω·cm²
        
        TLMResult() : slope(0.0), intercept(0.0), rSquared(0.0), sheetResistance(0.0), 
                     contactResistance(0.0), specificContactResistivity(0.0) {}
    };
    
    static bool linearRegression(const QVector<DataPoint> &dataPoints, TLMResult &result);
    static bool linearRegression(const QVector<double> &x, const QVector<double> &y, 
                                double &slope, double &intercept);
    static double calculateRSquared(const QVector<double> &x, const QVector<double> &y,
                                  double slope, double intercept);
    
private:
    Calculator() = default; // Static class, no instances
};

#endif // TLMANALYZER_CALCULATOR_H