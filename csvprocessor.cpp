#include "include/csvprocessor.h"
#include "include/datapoint.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

/**
 * @brief Process all CSV files in a folder to extract TLM data points
 * @param folderPath Path to the folder containing CSV files
 * @param config Application configuration
 * @return QVector of DataPoint objects extracted from the CSV files
 * 
 * This method scans a folder for CSV files, extracts spacing information from filenames,
 * processes each file to extract current/voltage measurements, and calculates resistance
 * values. The resulting data points are sorted by spacing before being returned.
 */
QVector<DataPoint> CSVProcessor::processFolder(const QString &folderPath, const AppConfig& config)
{
    QDir dir(folderPath);
    QStringList csvFiles = dir.entryList({"*.csv"}, QDir::Files);
    
    QVector<DataPoint> dataPoints;

    // Process each CSV file
    for (const QString &filename : csvFiles) {
        double spacing = extractSpacingFromFilename(filename);
        if (spacing > 0) {
            QString filePath = dir.filePath(filename);
            DataPoint point = processFile(filePath, config);
            
            if (point.resistance > 0 && std::isfinite(point.resistance)) {
                point.spacing = spacing;
                dataPoints.append(point);
                qDebug() << "File:" << filename << "Spacing:" << spacing << "μm, Resistance:" << point.resistance << "Ω, Current:" << point.current << "A";
            } else {
                qDebug() << "Skipping file due to invalid resistance:" << filename << point.resistance;
            }
        } else {
            qDebug() << "Filename does not contain valid spacing, skipping:" << filename;
        }
    }
    
    // Sort data points by spacing in ascending order
    std::sort(dataPoints.begin(), dataPoints.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.spacing < b.spacing;
    });
    
    return dataPoints;
}

/**
 * @brief Process a single CSV file to extract electrical measurements
 * @param filePath Path to the CSV file to process
 * @param config Application configuration
 * @return DataPoint object containing the extracted measurements
 * 
 * This method reads a CSV file containing electrical measurements, extracts voltage
 * and current values at the specified reference voltage and near zero volts, and
 * calculates the resistance. The method assumes a specific CSV format with voltage
 * in column 6 and current in column 7.
 */
DataPoint CSVProcessor::processFile(const QString &filePath, const AppConfig& config)
{
    DataPoint point;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << filePath;
        point.resistance = -1;
        return point;
    }

    QTextStream in(&file);
    bool foundVoltage = false;
    bool foundZero = false;
    double I_voltage = 0.0;
    double I_zero = 0.0;

    // Skip header (if present) and find data
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() >= 7) {
            bool ok1, ok2;
            double v = fields[5].toDouble(&ok1);  // Column 6 voltage
            double i = fields[6].toDouble(&ok2);  // Column 7 current

            if (ok1 && ok2) {
                if (std::abs(v - config.res_voltage) < 1e-3 && !foundVoltage) {
                    I_voltage = i;
                    foundVoltage = true;
                }
                if (std::abs(v) < 1e-3 && !foundZero) {
                    I_zero = i;
                    foundZero = true;
                }
            }
        }

        if (foundVoltage && foundZero) {
            break;
        }
    }

    file.close();

    if (foundVoltage && foundZero) {
        double deltaI = (I_voltage - I_zero);
        if (std::abs(deltaI) < 1e-15 || !std::isfinite(deltaI)) {
            point.resistance = -1;
            qDebug() << "Invalid current difference (zero or non-finite) in file:" << filePath << "deltaI=" << deltaI;
        } else {
            point.resistance = config.res_voltage / deltaI;
            point.current = deltaI;  // Store the current value
            qDebug() << "File:" << QFileInfo(filePath).fileName()
                     << "V:" << config.res_voltage << "I_voltage:" << I_voltage
                     << "I_zero:" << I_zero << "R:" << point.resistance;
        }
    } else {
        point.resistance = -1;
        qDebug() << "Incomplete data in file:" << filePath;
    }
    
    return point;
}

/**
 * @brief Extract spacing value from a filename
 * @param filename The filename to parse
 * @return The extracted spacing value, or -1 if no valid spacing was found
 * 
 * This method uses a regular expression to extract numeric spacing values from
 * filenames. It expects the spacing value to be the first number in the filename.
 */
double CSVProcessor::extractSpacingFromFilename(const QString &filename)
{
    static const QRegularExpression spacingRegex(R"((\d+(?:\.\d+)?))");
    QRegularExpressionMatch match = spacingRegex.match(QFileInfo(filename).baseName());
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    return -1;
}