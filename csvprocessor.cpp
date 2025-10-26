#include "include/csvprocessor.h"
#include "include/datapoint.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

QVector<DataPoint> CSVProcessor::processFolder(const QString &folderPath, double voltage)
{
    QDir dir(folderPath);
    QStringList csvFiles = dir.entryList({"*.csv"}, QDir::Files);
    
    QVector<DataPoint> dataPoints;

    // Process each CSV file
    for (const QString &filename : csvFiles) {
        double spacing = extractSpacingFromFilename(filename);
        if (spacing > 0) {
            QString filePath = dir.filePath(filename);
            DataPoint point = processFile(filePath, voltage);
            
            if (point.resistance > 0) {
                point.spacing = spacing;
                dataPoints.append(point);
                qDebug() << "File:" << filename << "Spacing:" << spacing << "μm, Resistance:" << point.resistance << "Ω, Current:" << point.current << "A";
            }
        }
    }
    
    // Sort data points by spacing in ascending order
    std::sort(dataPoints.begin(), dataPoints.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.spacing < b.spacing;
    });
    
    return dataPoints;
}

DataPoint CSVProcessor::processFile(const QString &filePath, double voltage)
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
                if (std::abs(v - voltage) < 1e-3 && !foundVoltage) {
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
        point.resistance = voltage / (I_voltage - I_zero);
        point.current = I_voltage - I_zero;  // Store the current value
        qDebug() << "File:" << QFileInfo(filePath).fileName()
                 << "V:" << voltage << "I_voltage:" << I_voltage
                 << "I_zero:" << I_zero << "R:" << point.resistance;
    } else {
        point.resistance = -1;
        qDebug() << "Incomplete data in file:" << filePath;
    }
    
    return point;
}

double CSVProcessor::extractSpacingFromFilename(const QString &filename)
{
    QRegularExpression spacingRegex(R"((\d+(?:\.\d+)?))");
    QRegularExpressionMatch match = spacingRegex.match(QFileInfo(filename).baseName());
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    return -1;
}