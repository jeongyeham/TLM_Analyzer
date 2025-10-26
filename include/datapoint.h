#ifndef TLMANALYZER_DATAPOINT_H
#define TLMANALYZER_DATAPOINT_H

#include <QMetaType>

struct DataPoint {
    double spacing;
    double resistance;
    double current;
    bool enabled;
    
    DataPoint() : spacing(0.0), resistance(0.0), current(0.0), enabled(true) {}
    
    DataPoint(double s, double r, double c, bool e = true) 
        : spacing(s), resistance(r), current(c), enabled(e) {}
};

// Required for Qt signal/slot mechanism
Q_DECLARE_METATYPE(DataPoint)
Q_DECLARE_METATYPE(QVector<DataPoint>)

#endif // TLMANALYZER_DATAPOINT_H