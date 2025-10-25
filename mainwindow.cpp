//
// Created by jeong on 2025/10/25.
//
#include "mainwindow.h"
#include "tlmanalyzer.h"
#include <QMenuBar>
#include <QMenu>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QGroupBox>

#include <QTimer>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QListWidget>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , analyzer(new TLMAnalyzer(this))
{
    setupMenu();
    setupUI();
    setupConnections();
    setupChart();

    setWindowTitle("TLM Analyzer - Qt6 Charts");
    resize(1200, 900);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenu()
{
    // Set up actions
    aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    
    // 创建菜单栏
    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // 文件夹选择区域
    QGroupBox *folderGroup = new QGroupBox("Data Selection");
    QHBoxLayout *folderLayout = new QHBoxLayout(folderGroup);
    folderPathEdit = new QLineEdit();
    folderPathEdit->setPlaceholderText("Select folder containing CSV files...");
    browseButton = new QPushButton("Browse...");
    folderLayout->addWidget(folderPathEdit);
    folderLayout->addWidget(browseButton);

    // 参数设置区域
    QGroupBox *paramGroup = new QGroupBox("Analysis Parameters");
    QGridLayout *paramLayout = new QGridLayout(paramGroup);

    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
    doubleValidator->setBottom(0);

    voltageEdit = new QLineEdit("1.0");
    voltageEdit->setValidator(doubleValidator);

    paramLayout->addWidget(new QLabel("Resistance Voltage (V):"), 0, 0);
    paramLayout->addWidget(voltageEdit, 0, 1);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    analyzeButton = new QPushButton("Analyze Data");
    analyzeButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; }");
    exportButton = new QPushButton("Export Plot");
    exportButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    buttonLayout->addWidget(analyzeButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addStretch();

    // 进度条
    progressBar = new QProgressBar();
    progressBar->setVisible(false);

    // 数据点管理区域
    QGroupBox *dataPointGroup = new QGroupBox("Data Points Management");
    QHBoxLayout *dataPointLayout = new QHBoxLayout(dataPointGroup);
    
    dataPointList = new QListWidget();
    dataPointList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QVBoxLayout *dataPointButtonLayout = new QVBoxLayout();
    addPointButton = new QPushButton("Add Point");
    removePointButton = new QPushButton("Remove Point");
    removePointButton->setEnabled(false);
    dataPointButtonLayout->addWidget(addPointButton);
    dataPointButtonLayout->addWidget(removePointButton);
    dataPointButtonLayout->addStretch();
    
    dataPointLayout->addWidget(dataPointList, 1);
    dataPointLayout->addLayout(dataPointButtonLayout);

    // 图表区域
    QGroupBox *chartGroup = new QGroupBox("TLM Analysis Plot");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);
    chartView = new QChartView();
    chartView->setMinimumSize(800, 500);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(chartView);

    // 结果区域
    QGroupBox *resultGroup = new QGroupBox("Analysis Results");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    resultText = new QTextEdit();
    resultText->setReadOnly(true);
    resultText->setMaximumHeight(250);
    resultText->setStyleSheet("QTextEdit { background-color: #f5f5f5; font-family: monospace; }");
    resultLayout->addWidget(resultText);

    // 组装主布局 (移除分割器)
    mainLayout->addWidget(folderGroup);
    mainLayout->addWidget(paramGroup);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(dataPointGroup);
    mainLayout->addWidget(chartGroup);
    mainLayout->addWidget(resultGroup);
}

void MainWindow::setupConnections()
{
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::selectFolder);
    connect(analyzeButton, &QPushButton::clicked, this, &MainWindow::analyzeData);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportPlot);
    connect(addPointButton, &QPushButton::clicked, this, &MainWindow::addDataPoint);
    connect(removePointButton, &QPushButton::clicked, this, &MainWindow::removeDataPoint);
    connect(dataPointList, &QListWidget::itemSelectionChanged, this, &MainWindow::onDataPointSelectionChanged);

    connect(analyzer.data(), &TLMAnalyzer::analysisComplete, this, &MainWindow::onAnalysisComplete);
    connect(analyzer.data(), &TLMAnalyzer::plotDataReady, this, &MainWindow::onPlotDataReady);
}

void MainWindow::setupChart()
{
    chart = new QChart();
    chart->setTitle("TLM Analysis - Resistance vs Pad Spacing");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chartView->setChart(chart);
}

void MainWindow::selectFolder()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        "Select Data Folder Containing CSV Files",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!folder.isEmpty()) {
        folderPathEdit->setText(folder);
        currentFolder = folder;

        // 自动扫描文件夹中的CSV文件
        QDir dir(folder);
        QStringList csvFiles = dir.entryList({"*.csv"}, QDir::Files);
        if (!csvFiles.isEmpty()) {
            QString fileInfo = QString("Found %1 CSV files:\n").arg(csvFiles.size());
            for (const QString &file : csvFiles) {
                fileInfo += "  • " + file + "\n";
            }
            resultText->setText(fileInfo);
        } else {
            resultText->setText("No CSV files found in the selected folder.");
        }
    }
}

void MainWindow::analyzeData()
{
    if (currentFolder.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a data folder first.");
        return;
    }

    bool ok;
    double voltage = voltageEdit->text().toDouble(&ok);
    if (!ok || voltage == 0) {
        QMessageBox::warning(this, "Warning", "Please enter a valid non-zero voltage.");
        return;
    }

    progressBar->setVisible(true);
    progressBar->setValue(0);
    analyzeButton->setEnabled(false);

    analyzer->analyzeFolder(currentFolder, voltage);
}

void MainWindow::exportPlot()
{
    if (!chart->series().isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Save Plot as PNG",
            "TLM_Analysis_Plot.png",
            "PNG Images (*.png)"
        );

        if (!fileName.isEmpty()) {
            if (!fileName.endsWith(".png", Qt::CaseInsensitive)) {
                fileName += ".png";
            }

            QPixmap pixmap = chartView->grab();
            if (pixmap.save(fileName, "PNG")) {
                QMessageBox::information(this, "Success",
                    QString("Plot exported successfully to:\n%1").arg(fileName));
            } else {
                QMessageBox::warning(this, "Error", "Failed to export plot.");
            }
        }
    } else {
        QMessageBox::warning(this, "Warning", "No plot data available to export.");
    }
}

void MainWindow::addDataPoint()
{
    // 弹出对话框让用户输入新的数据点
    bool ok1, ok2;
    double spacing = QInputDialog::getDouble(this, "Add Data Point", "Enter spacing value (μm):", 0, -1000000, 1000000, 3, &ok1);
    if (!ok1) return;
    
    double current = QInputDialog::getDouble(this, "Add Data Point", "Enter current value (A):", 0, -1000000, 1000000, 6, &ok2);
    if (!ok2) return;
    
    // 获取当前设置的电阻电压值
    bool voltageOk;
    double voltage = voltageEdit->text().toDouble(&voltageOk);
    if (!voltageOk || voltage <= 0) {
        QMessageBox::warning(this, "Invalid Voltage", "Please enter a valid resistance voltage value greater than zero.");
        return;
    }
    
    // 检查电流是否为0，避免除以0
    if (qFuzzyIsNull(current)) {
        QMessageBox::warning(this, "Invalid Input", "Current value cannot be zero.");
        return;
    }
    
    // 计算电阻值
    double resistance = voltage / current;
    
    // 添加到数据中
    originalSpacings.append(spacing);
    originalResistances.append(resistance);
    dataPointEnabled.append(true);
    
    // 更新列表和图表
    updateDataPointList();
    
    // 重新计算并绘制
    onPlotDataReady(originalSpacings, originalResistances, currentSlope, currentIntercept);
}

void MainWindow::removeDataPoint()
{
    int selectedIndex = dataPointList->currentRow();
    if (selectedIndex >= 0 && selectedIndex < dataPointEnabled.size()) {
        // 禁用选中的数据点
        dataPointEnabled[selectedIndex] = false;
        
        // 更新列表
        updateDataPointList();
        
        // 重新计算并绘制
        onPlotDataReady(originalSpacings, originalResistances, currentSlope, currentIntercept);
    }
}

void MainWindow::showAbout()
{
    QDialog aboutDialog(this);
    aboutDialog.setWindowTitle("About TLM Analyzer");
    
    QVBoxLayout *layout = new QVBoxLayout(&aboutDialog);
    
    QTextBrowser *textBrowser = new QTextBrowser();
    textBrowser->setOpenExternalLinks(true);
    textBrowser->setHtml(
        "<h2>TLM Analyzer</h2>"
        "<p><b>Version:</b> 2.0</p>"
        "<p><b>Description:</b> TLM Analyzer is a specialized tool for analyzing Transmission Line Model data from CSV files.</p>"
        "<p><b>Written by JeongYeham implementing Qt6.</b></p>"
        "<p><b>Special THANKS to Pudd1ng!!</b></p>"
        "<p><b>Technology:</b></p>"
        "<ul>"
        "<li>Qt6 Framework</li>"
        "<li>Qt Charts Module</li>"
        "</ul>"
        "<p><b>License:</b> MIT License</p>"
    );
    
    layout->addWidget(textBrowser);
    
    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, &aboutDialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    aboutDialog.setLayout(layout);
    aboutDialog.resize(400, 300);
    aboutDialog.exec();
}

void MainWindow::onAnalysisComplete(const QString &result)
{
    resultText->setText(result);
    progressBar->setValue(100);
    analyzeButton->setEnabled(true);

    // 进度条完成后隐藏
    QTimer::singleShot(2000, this, [this]() {
        progressBar->setVisible(false);
        progressBar->setValue(0);
    });
}

void MainWindow::onPlotDataReady(const QVector<double> &spacings,
                               const QVector<double> &resistances,
                               double slope, double intercept)
{
    // 存储原始数据
    originalSpacings = spacings;
    originalResistances = resistances;
    currentSlope = slope;
    currentIntercept = intercept;
    
    // 初始化数据点启用状态
    dataPointEnabled.resize(spacings.size());
    for (int i = 0; i < dataPointEnabled.size(); ++i) {
        dataPointEnabled[i] = true;
    }
    
    // 更新数据点列表
    updateDataPointList();
    
    // 清除之前的系列和坐标轴
    chart->removeAllSeries();
    chart->axes().clear(); // 清除所有坐标轴

    // 收集启用的数据点
    QVector<double> enabledSpacings, enabledResistances;
    for (int i = 0; i < originalSpacings.size(); ++i) {
        if (dataPointEnabled[i]) {
            enabledSpacings.append(originalSpacings[i]);
            enabledResistances.append(originalResistances[i]);
        }
    }

    if (enabledSpacings.isEmpty()) {
        resultText->setText("No data points available for analysis.");
        return;
    }

    // 创建散点系列用于测量数据
    QScatterSeries *scatterSeries = new QScatterSeries();
    scatterSeries->setName("Measured Data");
    scatterSeries->setMarkerSize(12);
    scatterSeries->setColor(QColor(255, 0, 0));
    scatterSeries->setBorderColor(QColor(200, 0, 0));

    // 创建线系列用于拟合
    QLineSeries *lineSeries = new QLineSeries();
    lineSeries->setName("Linear Fit");
    lineSeries->setColor(QColor(0, 0, 255));
    lineSeries->setPen(QPen(QBrush(QColor(0, 0, 255)), 2));

    // 添加数据点
    double minX = enabledSpacings[0], maxX = enabledSpacings[0];
    double minY = enabledResistances[0], maxY = enabledResistances[0];

    for (qsizetype i = 0; i < enabledSpacings.size(); ++i) {
        scatterSeries->append(enabledSpacings[i], enabledResistances[i]);

        // 更新范围
        if (enabledSpacings[i] < minX) minX = enabledSpacings[i];
        if (enabledSpacings[i] > maxX) maxX = enabledSpacings[i];
        if (enabledResistances[i] < minY) minY = enabledResistances[i];
        if (enabledResistances[i] > maxY) maxY = enabledResistances[i];
    }

    // 添加拟合线数据点（扩展范围以获得更好的视觉效果）
    double extendedMinX = minX - (maxX - minX) * 0.1;
    double extendedMaxX = maxX + (maxX - minX) * 0.1;

    lineSeries->append(extendedMinX, currentSlope * extendedMinX + currentIntercept);
    lineSeries->append(extendedMaxX, currentSlope * extendedMaxX + currentIntercept);

    // 添加系列到图表
    chart->addSeries(scatterSeries);
    chart->addSeries(lineSeries);

    // 配置坐标轴
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();

    axisX->setTitleText("Pad Spacing (μm)");
    axisY->setTitleText("Total Resistance (Ω)");
    axisX->setLabelFormat("%.1f");
    axisY->setLabelFormat("%.2f");

    // 设置坐标轴范围
    axisX->setRange(minX - (maxX - minX) * 0.05, maxX + (maxX - minX) * 0.05);
    axisY->setRange(minY - (maxY - minY) * 0.05, maxY + (maxY - minY) * 0.05);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    scatterSeries->attachAxis(axisX);
    scatterSeries->attachAxis(axisY);
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    // 更新图表标题以包含拟合信息
    chart->setTitle(QString("TLM Analysis - R = %1 × L + %2")
                   .arg(currentSlope, 0, 'f', 4)
                   .arg(currentIntercept, 0, 'f', 4));
}

void MainWindow::onDataPointSelectionChanged()
{
    removePointButton->setEnabled(!dataPointList->selectedItems().isEmpty());
}

void MainWindow::updateDataPointList()
{
    dataPointList->clear();
    
    for (int i = 0; i < originalSpacings.size(); ++i) {
        QString itemText = QString("Point %1: Spacing=%2 μm, Resistance=%3 Ω")
                          .arg(i + 1)
                          .arg(originalSpacings[i], 0, 'f', 3)
                          .arg(originalResistances[i], 0, 'f', 3);
        
        if (!dataPointEnabled[i]) {
            itemText += " (Removed)";
        }
        
        QListWidgetItem *item = new QListWidgetItem(itemText);
        if (!dataPointEnabled[i]) {
            item->setForeground(QColor(128, 128, 128)); // 灰色显示已移除的点
        }
        dataPointList->addItem(item);
    }
}