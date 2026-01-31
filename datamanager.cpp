#include "include/datamanager.h"
#include <algorithm>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMetaObject>
#include <QDir>
#include "include/csvprocessor.h"


/**
 * @brief Constructor for DataManager
 * @param parent The parent QObject
 * 
 * Initializes the DataManager with an empty data points collection.
 */
DataManager::DataManager(QObject *parent)
    : QObject(parent), m_channelWidth(100.0), m_resistanceVoltage(1.0), m_appConfig("config.json")
{
    m_model = new DataPointModel(this);

    m_loadWatcher = new QFutureWatcher<QVector<DataPoint>>(this);
    connect(m_loadWatcher, &QFutureWatcher<QVector<DataPoint>>::finished, this, [this]() {
        // Move results into the dataPoints and notify
        QVector<DataPoint> results = m_loadWatcher->result();
        dataPoints = results;
        sortDataPoints();
        if (m_model) m_model->setDataPoints(dataPoints);
        emit dataChanged();
        emit progressUpdated(100);
        m_cancelRequested.storeRelease(0);
    });
}

// Property accessors

QAbstractListModel* DataManager::model() const
{
    return m_model;
}

double DataManager::getChannelWidth() const
{
    return m_channelWidth;
}

void DataManager::setChannelWidth(double width)
{
    if (m_channelWidth != width) {
        m_channelWidth = width;
        
        // Save to configuration
        m_appConfig.setChannelLength(width);
        
        emit channelWidthChanged();
    }
}

double DataManager::getResistanceVoltage() const
{
    return m_resistanceVoltage;
}

void DataManager::setResistanceVoltage(double voltage)
{
    if (m_resistanceVoltage != voltage) {
        m_resistanceVoltage = voltage;
        
        // Save to configuration
        m_appConfig.setResistanceVoltage(voltage);
        
        emit resistanceVoltageChanged();
    }
}

/**
 * @brief Add a data point to the collection
 * @param point The DataPoint to add
 * 
 * Appends a new data point to the internal collection, sorts the data points
 * by spacing, and emits the dataChanged signal to notify any observers.
 */
void DataManager::c_addDataPoint(const DataPoint &point)
{
    dataPoints.append(point);
    sortDataPoints();
    if (m_model) m_model->setDataPoints(dataPoints);
    emit dataChanged();
}

/**
 * @brief Remove a data point at the specified index
 * @param index The index of the data point to remove
 * 
 * Removes the data point at the specified index from the internal collection
 * and emits the dataChanged signal if the index is valid.
 */
void DataManager::c_removeDataPoint(int index)
{
    if (index >= 0 && index < dataPoints.size()) {
        dataPoints.remove(index);
        if (m_model) m_model->setDataPoints(dataPoints);
        emit dataChanged();
    }
}

/**
 * @brief Set the enabled state of a data point
 * @param index The index of the data point to modify
 * @param enabled The new enabled state
 * 
 * Updates the enabled state of the data point at the specified index and
 * emits the dataChanged signal if the index is valid.
 */
void DataManager::c_setDataPointEnabled(int index, bool enabled)
{
    if (index >= 0 && index < dataPoints.size()) {
        dataPoints[index].enabled = enabled;
        if (m_model) m_model->setDataPoints(dataPoints);
        emit dataChanged();
    }
}

/**
 * @brief Clear all data points from the collection
 * 
 * Removes all data points from the internal collection and emits the
 * dataChanged signal.
 */
void DataManager::c_clearDataPoints()
{
    dataPoints.clear();
    if (m_model) m_model->setDataPoints(dataPoints);
    emit dataChanged();
}

/**
 * @brief Remove all disabled data points from the collection
 * 
 * Creates a new collection containing only enabled data points, replaces
 * the internal collection with this filtered collection, sorts the data
 * points by spacing, and emits the dataChanged signal.
 */
void DataManager::c_clearDisabledDataPoints()
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
    if (m_model) m_model->setDataPoints(dataPoints);
    emit dataChanged();
}

/**
 * @brief Get a reference to the internal collection of data points
 * @return A const reference to the internal QVector of DataPoint objects
 */
const QVector<DataPoint>& DataManager::getDataPoints() const
{
    return dataPoints;
}

/**
 * @brief Sort data points by spacing in ascending order
 * 
 * Sorts the internal collection of data points by their spacing values
 * in ascending order using std::sort with a lambda comparison function.
 */
void DataManager::sortDataPoints()
{
    // Sort data points by spacing in ascending order
    std::sort(dataPoints.begin(), dataPoints.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.spacing < b.spacing;
    });
}

/**
 * @brief Get a collection of only the enabled data points
 * @return A QVector containing only the enabled data points
 */
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

/**
 * @brief Add a manually entered data point
 * @param spacing The spacing value for the new data point (in μm)
 * @param current The current value for the new data point (in A)
 * @param voltage The voltage value for calculating resistance (in V)
 * 
 * Calculates resistance from the provided voltage and current values,
 * creates a new DataPoint with the provided values, appends it to the
 * internal collection, sorts the data points by spacing, and emits the
 * dataChanged signal.
 */
void DataManager::c_addManualDataPoint(double spacing, double current, double voltage)
{
    // Calculate resistance from voltage and current
    double resistance = 0.0;
    double deltaI = current;
    if (std::abs(deltaI) < 1e-15 || !std::isfinite(deltaI)) {
        resistance = -1;
    } else {
        resistance = voltage / deltaI;
    }

    DataPoint point(spacing, resistance, current, true);
    dataPoints.append(point);
    sortDataPoints();
    if (m_model) m_model->setDataPoints(dataPoints);
    emit dataChanged();
}

/**
 * @brief Load data from a folder of CSV files
 * @param folderPath Path to the folder containing CSV files
 * @param voltage Reference voltage for resistance calculations
 */
void DataManager::c_loadDataFromFolder(const QString &folderPath, double voltage)
{
    // If a load is already in progress, refuse to start another
    if (m_loadWatcher && m_loadWatcher->isRunning()) {
        emit analysisComplete("Loading already in progress.");
        return;
    }

    m_cancelRequested.storeRelease(0);

    setCurrentFolder(folderPath);
    setResistanceVoltage(voltage);  // Update the resistance voltage in persistent config

    // Prepare local copies for the background task
    AppConfig configCopy = m_appConfig; // copy configuration for thread safety

    QDir dir(folderPath);
    QStringList csvFiles = dir.entryList({"*.csv"}, QDir::Files);

    // Launch background task that processes files one-by-one and reports progress
    QFuture<QVector<DataPoint>> future = QtConcurrent::run([folderPath, csvFiles, configCopy, this]() -> QVector<DataPoint> {
        QVector<DataPoint> points;
        qsizetype total = csvFiles.size();
        for (qsizetype i = 0; i < total; ++i) {
            if (m_cancelRequested.loadAcquire() != 0) {
                // Cancel requested; stop processing
                QMetaObject::invokeMethod(this, "emitProgress", Qt::QueuedConnection, Q_ARG(int, 0));
                return points;
            }

            const QString &filename = csvFiles.at(static_cast<int>(i));
            QString path = QDir(folderPath).filePath(filename);

            DataPoint p = CSVProcessor::processFile(path, configCopy);
            if (p.resistance > 0 && std::isfinite(p.resistance)) {
                double spacing = CSVProcessor::extractSpacingFromFilename(filename);
                if (spacing > 0) {
                    p.spacing = spacing;
                    points.append(p);
                }
            }

            int percent = total > 0 ? static_cast<int>(((i + 1) * 100) / total) : 100;
            // Safely invoke progress emission in the GUI thread
            QMetaObject::invokeMethod(this, "emitProgress", Qt::QueuedConnection, Q_ARG(int, percent));
        }

        // Sort here to keep behavior consistent
        std::sort(points.begin(), points.end(), [](const DataPoint& a, const DataPoint& b) {
            return a.spacing < b.spacing;
        });

        return points;
    });

    // Set future to watcher so finished() handler will pick up results
    m_loadWatcher->setFuture(future);

    Q_UNUSED(csvFiles);
}

void DataManager::c_cancelLoad()
{
    if (m_loadWatcher && m_loadWatcher->isRunning()) {
        m_cancelRequested.storeRelease(1);
    }
}

/**
 * @brief Perform TLM analysis on the current data points
 * @param channelWidth Width of the channel in μm
 */
void DataManager::c_performAnalysis(double channelWidth)
{
    if (channelWidth <= 0 || !std::isfinite(channelWidth)) {
        emit analysisComplete("Invalid channel width specified.");
        return;
    }

    setChannelWidth(channelWidth);
    
    Calculator::TLMResult result;
    if (calculateTLMResults(result, channelWidth)) {
        // Store last result for QML access
        m_lastResult = result;

        // Format the result message
        QString resultMessage = QString(
            "TLM Analysis Results:\n"
            "====================\n"
            "Sheet Resistance: %1 Ω/sq\n"
            "Contact Resistance: %2 Ω\n"
            "Specific Contact Resistivity: %3 Ω·cm²\n"
            "Linear Fit Slope: %4 Ω/μm\n"
            "Linear Fit Intercept: %5 Ω\n"
            "R² (Goodness of Fit): %6\n"
            "Channel Width: %7 μm")
            .arg(result.sheetResistance)
            .arg(result.contactResistance)
            .arg(result.specificContactResistivity)
            .arg(result.slope)
            .arg(result.intercept)
            .arg(result.rSquared)
            .arg(result.channelWidth);
        
        emit analysisComplete(resultMessage);
    } else {
        emit analysisComplete("Analysis failed. Please check your data.");
    }
}

QVariantMap DataManager::c_lastAnalysisResult() const
{
    QVariantMap map;
    map["slope"] = m_lastResult.slope;
    map["intercept"] = m_lastResult.intercept;
    map["rSquared"] = m_lastResult.rSquared;
    map["sheetResistance"] = m_lastResult.sheetResistance;
    map["contactResistance"] = m_lastResult.contactResistance;
    map["specificContactResistivity"] = m_lastResult.specificContactResistivity;
    map["channelWidth"] = m_lastResult.channelWidth;
    return map;
}

/**
 * @brief Get the number of data points in the collection
 * @return The size of the internal data points collection
 */
qsizetype DataManager::size() const
{
    return dataPoints.size();
}

/**
 * @brief Access a data point at the specified index
 * @param index The index of the data point to access
 * @return A const reference to the data point at the specified index
 */
const DataPoint& DataManager::at(qsizetype index) const
{
    return dataPoints.at(index);
}

/**
 * @brief Calculate TLM results using linear regression with specified channel width
 * @param result Reference to a TLMResult object to store the calculation results
 * @param channelWidth Width of the channel in μm
 * @return True if the calculation was successful, false otherwise
 * 
 * Performs linear regression analysis on the data points and stores the
 * results in the provided TLMResult object.
 */
bool DataManager::calculateTLMResults(Calculator::TLMResult &result, double channelWidth) const
{
    return Calculator::linearRegression(dataPoints, result, channelWidth);
}

/**
 * @brief Emit progress update for loading data
 * @param progress The progress percentage (0-100)
 *
 * Emits the progressUpdated signal with the specified progress value.
 */
void DataManager::emitProgress(int progress)
{
    emit progressUpdated(progress);
}
