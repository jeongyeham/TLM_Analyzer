#ifndef RC0_01_MAINWINDOW_H
#define RC0_01_MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QVector>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>

// Forward declarations
QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLineEdit;
class QLabel;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class QAction;
class QListWidget;
QT_END_NAMESPACE

class DataManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void selectFolder();
    void analyzeData();
    void exportPlot();
    void addDataPoint();
    void removeDataPoint() const;
    void clearDisabledPoints() const;
    void showAbout();
    void onPlotDataReady(const QVector<double> &spacings,
                        const QVector<double> &resistances,
                        const QVector<double> &currents,
                        double slope, double intercept);
    void onDataPointSelectionChanged() const;
    void onDataChanged();

private:
    void setupUI();
    void setupMenu();
    void setupConnections();
    void setupChart();
    void updateDataPointList() const;

    // Use smart pointer to manage object lifetime
    // QScopedPointer<TLMAnalyzer> analyzer; // Remove this legacy analyzer

    // Data manager
    DataManager* dataManager;

    // UI Components
    QWidget *centralWidget{};
    QVBoxLayout *mainLayout{};

    QLineEdit *folderPathEdit{};
    QPushButton *browseButton{};

    QPushButton *analyzeButton{};
    QPushButton *exportButton{};

    QPushButton *addPointButton{};
    QPushButton *removePointButton{};
    QPushButton *clearDisabledPointsButton{};

    QLineEdit *voltageEdit{};

    QTextEdit *resultText{};

    QProgressBar *progressBar{};

    QListWidget *dataPointList{};

    QChartView *chartView{};
    QChart *chart{};

    QString currentFolder;
    
    // Menu actions
    QAction *aboutAction{};
    
    // Chart text items
    QGraphicsRectItem *backgroundRect{};
    QGraphicsSimpleTextItem *textItem{};
};

#endif //RC0_01_MAINWINDOW_H