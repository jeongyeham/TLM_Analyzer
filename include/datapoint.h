#ifndef TLMANALYZER_DATAPOINT_H
#define TLMANALYZER_DATAPOINT_H

#include <QMetaType>
#include <QVector>


/**
 * @brief Represents a single data point in TLM analysis
 * 
 * Contains the spacing, resistance, current measurements and enabled status
 * for a single TLM data point.
 */
struct DataPoint {
    double spacing;      ///< Pad spacing in micrometers (μm)
    double resistance;   ///< Total resistance in ohms (Ω)
    double current;      ///< Current in amperes (A)
    bool enabled;        ///< Whether this data point is enabled for analysis
    
    /**
     * @brief Default constructor
     * Initializes all values to zero, with enabled set to true
     */
    DataPoint() : spacing(0.0), resistance(0.0), current(0.0), enabled(true) {}
    
    /**
     * @brief Parameterized constructor
     * @param s Spacing value
     * @param r Resistance value
     * @param c Current value
     * @param e Enabled status (default: true)
     */
    DataPoint(double s, double r, double c, bool e = true) 
        : spacing(s), resistance(r), current(c), enabled(e) {}
};

// Required for Qt signal/slot mechanism
Q_DECLARE_METATYPE(DataPoint)
Q_DECLARE_METATYPE(QVector<DataPoint>)

#endif // TLMANALYZER_DATAPOINT_H