#ifndef TLMANALYZER_TLMANALYZER_H
#define TLMANALYZER_TLMANALYZER_H

#include <QVector>
#include <QString>
#include <QRegularExpression>

class TLMAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit TLMAnalyzer(QObject *parent = nullptr);

public slots:
    void analyzeFolder(const QString &folderPath, double voltage);

signals:
    void analysisComplete(const QString &result);
    void plotDataReady(const QVector<double> &spacings,
                      const QVector<double> &resistances,
                      double slope, double intercept);

private:
    static double getResistance(const QString &filePath, double voltage);
    double extractSpacingFromFilename(const QString &filename) const;

    static bool linearRegression(const QVector<double> &x, const QVector<double> &y,
                                 double &slope, double &intercept);

    QRegularExpression spacingRegex;
};

#endif //TLMANALYZER_TLMANALYZER_H