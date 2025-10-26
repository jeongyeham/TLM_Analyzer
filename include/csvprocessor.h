#ifndef TLMANALYZER_CSVPARSER_H
#define TLMANALYZER_CSVPARSER_H

#include <QString>
#include <QVector>
#include "datapoint.h"

class CSVProcessor {
public:
    static QVector<DataPoint> processFolder(const QString &folderPath, double voltage);
    static DataPoint processFile(const QString &filePath, double voltage);
    static double extractSpacingFromFilename(const QString &filename);

private:
    CSVProcessor() = default; // Static class
};

#endif // TLMANALYZER_CSVPARSER_H