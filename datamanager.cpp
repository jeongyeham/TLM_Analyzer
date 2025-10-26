#include "include/datamanager.h"
#include <algorithm>

DataManager::DataManager(QObject *parent)
    : QObject(parent)
{
}

void DataManager::addDataPoint(const DataPoint &point)
{
    dataPoints.append(point);
    sortDataPoints();
    emit dataChanged();
}

void DataManager::removeDataPoint(int index)
{
    if (index >= 0 && index < dataPoints.size()) {
        dataPoints.remove(index);
        emit dataChanged();
    }
}

void DataManager::setDataPointEnabled(int index, bool enabled)
{
    if (index >= 0 && index < dataPoints.size()) {
        dataPoints[index].enabled = enabled;
        emit dataChanged();
    }
}

void DataManager::clearDataPoints()
{
    dataPoints.clear();
    emit dataChanged();
}

void DataManager::clearDisabledDataPoints()
{
    // Create a new vector with only enabled data points
    QVector<DataPoint> enabledPoints;
    for (const DataPoint &point : dataPoints) {
        if (point.enabled) {
            enabledPoints.append(point);
        }
    }
    
    // Replace the old data with only enabled points
    dataPoints = enabledPoints;
    sortDataPoints();
    emit dataChanged();
}

const QVector<DataPoint>& DataManager::getDataPoints() const
{
    return dataPoints;
}

void DataManager::sortDataPoints()
{
    // Sort data points by spacing in ascending order
    std::sort(dataPoints.begin(), dataPoints.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.spacing < b.spacing;
    });
}

QVector<DataPoint> DataManager::getEnabledDataPoints() const
{
    QVector<DataPoint> enabledPoints;
    for (const DataPoint &point : dataPoints) {
        if (point.enabled) {
            enabledPoints.append(point);
        }
    }
    return enabledPoints;
}

void DataManager::addManualDataPoint(double spacing, double current, double voltage)
{
    // Calculate resistance from voltage and current
    double resistance = voltage / current;
    
    DataPoint point(spacing, resistance, current, true);
    dataPoints.append(point);
    sortDataPoints();
    emit dataChanged();
}

int DataManager::size() const
{
    return dataPoints.size();
}

const DataPoint& DataManager::at(int index) const
{
    return dataPoints.at(index);
}

bool DataManager::calculateTLMResults(Calculator::TLMResult &result) const
{
    return Calculator::linearRegression(dataPoints, result);
}