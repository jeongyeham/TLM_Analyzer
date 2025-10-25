//
// Created by jeong on 2025/10/25.
//
#include "tlmanalyzer.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <algorithm>
#include <cmath>

TLMAnalyzer::TLMAnalyzer(QObject *parent)
    : QObject(parent)
{
    // Regular expression to extract spacing from filename
    spacingRegex.setPattern(R"((\d+(?:\.\d+)?))");
}

void TLMAnalyzer::analyzeFolder(const QString &folderPath, double voltage)
{
    QDir dir(folderPath);
    QStringList csvFiles = dir.entryList({"*.csv"}, QDir::Files);

    if (csvFiles.isEmpty()) {
        emit analysisComplete("No CSV files found in the selected folder.");
        return;
    }

    QVector<double> spacings;
    QVector<double> resistances;

    // Process each CSV file
    for (const QString &filename : csvFiles) {
        double spacing = extractSpacingFromFilename(filename);
        if (spacing > 0) {
            QString filePath = dir.filePath(filename);
            double resistance = getResistance(filePath, voltage);

            if (resistance > 0) {
                spacings.append(spacing);
                resistances.append(resistance);
                qDebug() << "File:" << filename << "Spacing:" << spacing << "μm, Resistance:" << resistance << "Ω";
            }
        }
    }

    if (spacings.size() < 2) {
        emit analysisComplete("Insufficient valid data points for analysis.\nNeed at least 2 valid measurements.");
        return;
    }

    // Sort by spacing
    QVector<qsizetype> indices(spacings.size());
    for (qsizetype i = 0; i < spacings.size(); ++i) indices[i] = i;

    std::sort(indices.begin(), indices.end(), [&](qsizetype a, qsizetype b) {
        return spacings[a] < spacings[b];
    });

    QVector<double> sortedSpacings, sortedResistances;
    for (qsizetype idx : indices) {
        sortedSpacings.append(spacings[idx]);
        sortedResistances.append(resistances[idx]);
    }

    // Perform linear regression
    double slope, intercept;
    if (!linearRegression(sortedSpacings, sortedResistances, slope, intercept)) {
        emit analysisComplete("Linear regression failed - data may be invalid.");
        return;
    }

    // Calculate parameters
    double Rsh = slope * 100.0;  // Convert to Ω/sq
    double Rc = intercept / 20.0; // Contact resistance
    double Rouc = (Rc * Rc / Rsh) * 1e-2; // Specific contact resistivity

    // Format results
    QString resultText = QString(
        "TLM Analysis Results:\n"
        "====================\n"
        "Sheet Resistance (Rsh): %1 Ω/sq\n"
        "Contact Resistance (Rc): %2 Ω·mm\n"
        "Specific Contact Resistivity (ρc): %3 Ω·cm²\n\n"
        "Linear Fit: R = %4 × L + %5\n\n"
        "Data Points:\n"
        "Spacing (μm) | Resistance (Ω)\n"
        "-----------------------------\n"
    ).arg(Rsh, 0, 'f', 3)
     .arg(Rc, 0, 'f', 3)
     .arg(Rouc, 0, 'e', 3)
     .arg(slope, 0, 'f', 6)
     .arg(intercept, 0, 'f', 6);

    for (qsizetype i = 0; i < sortedSpacings.size(); ++i) {
        resultText += QString("  %1\t\t%2\n").arg(sortedSpacings[i], 6, 'f', 1).arg(sortedResistances[i], 0, 'f', 6);
    }

    resultText += QString("\nFiles processed: %1").arg(csvFiles.size());

    emit analysisComplete(resultText);
    emit plotDataReady(sortedSpacings, sortedResistances, slope, intercept);
}

double TLMAnalyzer::getResistance(const QString &filePath, double voltage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << filePath;
        return -1;
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
        double resistance = voltage / (I_voltage - I_zero);
        qDebug() << "File:" << QFileInfo(filePath).fileName()
                 << "V:" << voltage << "I_voltage:" << I_voltage
                 << "I_zero:" << I_zero << "R:" << resistance;
        return resistance;
    }

    qDebug() << "Incomplete data in file:" << filePath;
    return -1;
}

double TLMAnalyzer::extractSpacingFromFilename(const QString &filename) const {
    QRegularExpressionMatch match = spacingRegex.match(QFileInfo(filename).baseName());
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    return -1;
}

bool TLMAnalyzer::linearRegression(const QVector<double> &x, const QVector<double> &y,
                                  double &slope, double &intercept)
{
    if (x.size() != y.size() || x.size() < 2) {
        return false;
    }

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    qsizetype n = x.size();

    for (qsizetype i = 0; i < n; ++i) {
        sumX += x[i];
        sumY += y[i];
        sumXY += x[i] * y[i];
        sumX2 += x[i] * x[i];
    }

    double denominator = n * sumX2 - sumX * sumX;
    if (std::abs(denominator) < 1e-12) {
        return false;
    }

    slope = (n * sumXY - sumX * sumY) / denominator;
    intercept = (sumY - slope * sumX) / n;

    return true;
}