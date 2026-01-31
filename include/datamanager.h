#ifndef TLMANALYZER_DATAMANAGER_H
#define TLMANALYZER_DATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QFutureWatcher>
#include "datapoint.h"
#include "calculator.h"
#include "appconfig.h"
#include "datapointmodel.h"

/**
 * @brief Manages TLM data points and provides an interface between C++ and QML
 * 
 * The DataManager class is responsible for storing, managing, and manipulating
 * Transmission Line Method (TLM) data points. It acts as a bridge between
 * the C++ backend and the QML frontend, providing methods for data manipulation 
 * and analysis.
 * 
 * The class maintains a collection of DataPoint objects and provides methods
 * to add, remove, modify, and analyze these data points. It also supports
 * data change notifications through Qt signals.
 */
class DataManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractListModel* model READ model NOTIFY dataChanged)
    Q_PROPERTY(QVector<DataPoint> dataPoints READ getDataPoints NOTIFY dataChanged)
    Q_PROPERTY(double resistanceVoltage READ getResistanceVoltage WRITE setResistanceVoltage NOTIFY resistanceVoltageChanged)
    Q_PROPERTY(double channelWidth READ getChannelWidth WRITE setChannelWidth NOTIFY channelWidthChanged)
public:
    /**
     * @brief Constructor for DataManager
     * @param parent The parent QObject
     */
    explicit DataManager(QObject *parent = nullptr);

    /**
     * @brief Add a data point to the collection
     * @param point The DataPoint to add
     * 
     * Appends a new data point to the internal collection, sorts the data points
     * by spacing, and emits the dataChanged signal to notify any observers.
     */
    Q_INVOKABLE void c_addDataPoint(const DataPoint &point);

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
    Q_INVOKABLE void c_addManualDataPoint(double spacing, double current, double voltage);
    
    /**
     * @brief Remove a data point at the specified index
     * @param index The index of the data point to remove
     * 
     * Removes the data point at the specified index from the internal collection
     * and emits the dataChanged signal if the index is valid.
     */
    Q_INVOKABLE void c_removeDataPoint(int index);
    
    /**
     * @brief Set the enabled state of a data point
     * @param index The index of the data point to modify
     * @param enabled The new enabled state
     * 
     * Updates the enabled state of the data point at the specified index and
     * emits the dataChanged signal if the index is valid.
     */
    Q_INVOKABLE void c_setDataPointEnabled(int index, bool enabled);
    
    /**
     * @brief Clear all data points from the collection
     * 
     * Removes all data points from the internal collection and emits the
     * dataChanged signal.
     */
    Q_INVOKABLE void c_clearDataPoints();
    
    /**
     * @brief Remove all disabled data points from the collection
     * 
     * Creates a new collection containing only enabled data points, replaces
     * the internal collection with this filtered collection, sorts the data
     * points by spacing, and emits the dataChanged signal.
     */
    Q_INVOKABLE void c_clearDisabledDataPoints();
    
    /**
     * @brief Load data from a folder of CSV files
     * @param folderPath Path to the folder containing CSV files
     * @param voltage Reference voltage for resistance calculations
     */
    Q_INVOKABLE void c_loadDataFromFolder(const QString &folderPath, double voltage);
    
    /**
     * @brief Perform TLM analysis on the current data points
     * @param channelWidth Width of the channel in μm
     */
    Q_INVOKABLE void c_performAnalysis(double channelWidth);
    
    // Return last analysis results (slope/intercept/rSquared/sheetResistance/contactResistance)
    Q_INVOKABLE QVariantMap c_lastAnalysisResult() const;

    [[nodiscard]] QAbstractListModel* model() const;

    [[nodiscard]] const QVector<DataPoint>& getDataPoints() const;
    [[nodiscard]] QVector<DataPoint> getEnabledDataPoints() const;
    [[nodiscard]] qsizetype size() const;
    [[nodiscard]] const DataPoint& at(qsizetype index) const;
    [[nodiscard]] QString currentFolder() const { return m_currentFolder; }

    /**
     * @brief Set current folder path
     * @param folderPath The new folder path
     */
    void setCurrentFolder(const QString& folderPath) { if (m_currentFolder != folderPath) { m_currentFolder = folderPath; emit currentFolderChanged(); } }

    /**
     * @brief Get channel width
     * @return Channel width value
     */
    Q_INVOKABLE double getChannelWidth() const;
    
    /**
     * @brief Set channel width
     * @param width The new channel width
     */
    void setChannelWidth(double width);

    /**
     * @brief Get resistance voltage
     * @return Resistance voltage value
     */
    Q_INVOKABLE double getResistanceVoltage() const;

    /**
     * @brief Set resistance voltage
     * @param voltage The new resistance voltage
     */
    void setResistanceVoltage(double voltage);
    
    /**
     * @brief Calculate TLM results using linear regression with specified channel width
     * @param result Reference to a TLMResult object to store the calculation results
     * @param channelWidth Width of the channel in μm
     * @return True if the calculation was successful, false otherwise
     * 
     * Performs linear regression analysis on the data points and stores the
     * results in the provided TLMResult object.
     */
    bool calculateTLMResults(Calculator::TLMResult &result, double channelWidth) const;

    // Cancel loading in progress
    Q_INVOKABLE void c_cancelLoad();

signals:
    /**
     * @brief Signal emitted when data changes
     * 
     * This signal is emitted whenever the data collection is modified,
     * notifying any connected components that they should update their displays.
     */
    void dataChanged();
    
    /**
     * @brief Signal emitted when current folder changes
     */
    void currentFolderChanged();
    
    /**
     * @brief Signal emitted when analysis results are ready
     */
    void analysisComplete(const QString& resultMessage);
    
    /**
     * @brief Signal emitted to update progress
     */
    void progressUpdated(int progress);
    
    /**
     * @brief Signal emitted when channel width changes
     */
    void channelWidthChanged();

    /**
     * @brief Signal emitted when resistance voltage changes
     */
    void resistanceVoltageChanged();

private slots:
    // Invokable helper to emit progress from background thread via invokeMethod
    void emitProgress(int progress);

private:
    /**
     * @brief Sort data points by spacing in ascending order
     * 
     * Sorts the internal collection of data points by their spacing values
     * in ascending order using std::sort with a lambda comparison function.
     */
    void sortDataPoints();
    
    QVector<DataPoint> dataPoints;  ///< Internal collection of data points
    QString m_currentFolder;        ///< Current folder path for CSV files
    double m_channelWidth;          ///< Channel width for calculations
    double m_resistanceVoltage;     ///< Resistance voltage for calculations
    AppConfig m_appConfig;          ///< Application configuration

    // Background loading watcher for asynchronous folder processing
    QFutureWatcher<QVector<DataPoint>> *m_loadWatcher = nullptr;

    Calculator::TLMResult m_lastResult;

    QAtomicInt m_cancelRequested {0};

    DataPointModel *m_model = nullptr;
};

#endif // TLMANALYZER_DATAMANAGER_H
