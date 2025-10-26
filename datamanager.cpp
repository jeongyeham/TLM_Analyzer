#include "include/datamanager.h"

DataManager::DataManager(QObject *parent)
    : QObject(parent)
{
}

void DataManager::addDataPoint(const DataPoint &point)
{
    dataPoints.append(point);
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

const QVector<DataPoint>& DataManager::getDataPoints() const
{
    return dataPoints;
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

bool DataManager::calculateTLMResults(Calculator::TLMResult &result) const
{
    return Calculator::linearRegression(dataPoints, result);
}