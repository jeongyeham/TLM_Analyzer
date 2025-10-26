#ifndef TLMANALYZER_DATAMANAGER_H
#define TLMANALYZER_DATAMANAGER_H

#include <QObject>
#include <QVector>
#include "datapoint.h"
#include "calculator.h"

class DataManager : public QObject
{
    Q_OBJECT

public:
    explicit DataManager(QObject *parent = nullptr);
    
    void addDataPoint(const DataPoint &point);
    void removeDataPoint(int index);
    void setDataPointEnabled(int index, bool enabled);
    void clearDataPoints();
    void clearDisabledDataPoints();
    
    const QVector<DataPoint>& getDataPoints() const;
    QVector<DataPoint> getEnabledDataPoints() const;
    
    void addManualDataPoint(double spacing, double current, double voltage);
    int size() const;
    const DataPoint& at(int index) const;
    
    bool calculateTLMResults(Calculator::TLMResult &result) const;
    
signals:
    void dataChanged();

private:
    void sortDataPoints();
    QVector<DataPoint> dataPoints;
};

#endif // TLMANALYZER_DATAMANAGER_H