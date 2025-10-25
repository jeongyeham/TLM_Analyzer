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
    
    // Create menu bar
    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // Folder selection area
    auto folderGroup = new QGroupBox("Data Selection");
    auto folderLayout = new QHBoxLayout(folderGroup);
    folderPathEdit = new QLineEdit();
    folderPathEdit->setPlaceholderText("Select folder containing CSV files...");
    browseButton = new QPushButton("Browse...");
    folderLayout->addWidget(folderPathEdit);
    folderLayout->addWidget(browseButton);

    // Parameter setting area
    auto paramGroup = new QGroupBox("Analysis Parameters");
    auto paramLayout = new QGridLayout(paramGroup);

    auto doubleValidator = new QDoubleValidator(this);
    doubleValidator->setBottom(0);

    voltageEdit = new QLineEdit("1.0");
    voltageEdit->setValidator(doubleValidator);

    paramLayout->addWidget(new QLabel("Resistance Voltage (V):"), 0, 0);
    paramLayout->addWidget(voltageEdit, 0, 1);

    // Button area
    auto buttonLayout = new QHBoxLayout();
    analyzeButton = new QPushButton("Analyze Data");
    analyzeButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; }");
    exportButton = new QPushButton("Export Plot");
    exportButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    buttonLayout->addWidget(analyzeButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addStretch();

    // Progress bar
    progressBar = new QProgressBar();
    progressBar->setVisible(false);

    // Data point management area
    auto dataPointGroup = new QGroupBox("Data Points Management");
    auto dataPointLayout = new QHBoxLayout(dataPointGroup);
    
    dataPointList = new QListWidget();
    dataPointList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    auto dataPointButtonLayout = new QVBoxLayout();
    addPointButton = new QPushButton("Add Point");
    removePointButton = new QPushButton("Remove Point");
    removePointButton->setEnabled(false);
    dataPointButtonLayout->addWidget(addPointButton);
    dataPointButtonLayout->addWidget(removePointButton);
    dataPointButtonLayout->addStretch();
    
    dataPointLayout->addWidget(dataPointList, 1);
    dataPointLayout->addLayout(dataPointButtonLayout);

    // Chart area
    auto chartGroup = new QGroupBox("TLM Analysis Plot");
    auto chartLayout = new QVBoxLayout(chartGroup);
    chartView = new QChartView();
    chartView->setMinimumSize(800, 500);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(chartView);

    // Results area
    auto resultGroup = new QGroupBox("Analysis Results");
    auto resultLayout = new QVBoxLayout(resultGroup);
    resultText = new QTextEdit();
    resultText->setReadOnly(true);
    resultText->setMaximumHeight(250);
    //resultText->setStyleSheet("QTextEdit { background-color: #f5f5f5; font-family: monospace; }");
    resultLayout->addWidget(resultText);

    // Assemble main layout (remove splitter)
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
    connect(analyzer.data(), &TLMAnalyzer::plotDataReady, this, [this](const QVector<double> &spacings,
                                                                      const QVector<double> &resistances,
                                                                      const QVector<double> &currents,
                                                                      double slope, double intercept) {
        this->onPlotDataReady(spacings, resistances, currents, slope, intercept);
    });
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

        // Automatically scan for CSV files in folder
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
    // Pop up dialog to let user input new data point
    bool ok1, ok2;
    double spacing = QInputDialog::getDouble(this, "Add Data Point", "Enter spacing value (μm):", 0, -1000000, 1000000, 3, &ok1);
    if (!ok1) return;
    
    double current = QInputDialog::getDouble(this, "Add Data Point", "Enter current value (A):", 0, -1000000, 1000000, 6, &ok2);
    if (!ok2) return;
    
    // Get the currently set resistance voltage value
    bool voltageOk;
    double voltage = voltageEdit->text().toDouble(&voltageOk);
    if (!voltageOk || voltage <= 0) {
        QMessageBox::warning(this, "Invalid Voltage", "Please enter a valid resistance voltage value greater than zero.");
        return;
    }
    
    // Check if current is 0 to avoid division by 0
    if (qFuzzyIsNull(current)) {
        QMessageBox::warning(this, "Invalid Input", "Current value cannot be zero.");
        return;
    }
    
    // Calculate resistance value
    double resistance = voltage / current;
    
    // Add to data
    originalSpacings.append(spacing);
    originalResistances.append(resistance);
    originalCurrents.append(current);  // Store current value
    dataPointEnabled.append(true);
    
    // Update list and chart
    updateDataPointList();
    
    // Recalculate and redraw
    // Create a temporary currents vector with zeros for existing points and the new current value
    QVector<double> tempCurrents = originalCurrents;
    onPlotDataReady(originalSpacings, originalResistances, tempCurrents, currentSlope, currentIntercept);
}

void MainWindow::removeDataPoint()
{
    int selectedIndex = dataPointList->currentRow();
    if (selectedIndex >= 0 && selectedIndex < dataPointEnabled.size()) {
        // Disable selected data point
        dataPointEnabled[selectedIndex] = false;
        
        // Update list
        updateDataPointList();
        
        // Recalculate and redraw
        // Create a temporary currents vector with zeros for existing points
        QVector<double> tempCurrents = originalCurrents;
        onPlotDataReady(originalSpacings, originalResistances, tempCurrents, currentSlope, currentIntercept);
    }
}

void MainWindow::showAbout()
{
    QDialog aboutDialog(this);
    aboutDialog.setWindowTitle("About TLM Analyzer");
    
    auto layout = new QVBoxLayout(&aboutDialog);
    
    auto textBrowser = new QTextBrowser();
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
    
    auto closeButton = new QPushButton("Close");
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

    // Hide progress bar when complete
    QTimer::singleShot(2000, this, [this]() {
        progressBar->setVisible(false);
        progressBar->setValue(0);
    });
}

void MainWindow::onPlotDataReady(const QVector<double> &spacings,
                               const QVector<double> &resistances,
                               const QVector<double> &currents,
                               double slope, double intercept)
{
    // Store original data
    originalSpacings = spacings;
    originalResistances = resistances;
    originalCurrents = currents;  // Store current values
    
    currentSlope = slope;
    currentIntercept = intercept;
    
    // Initialize data point enable status
    dataPointEnabled.resize(spacings.size());
    for (bool & i : dataPointEnabled) {
        i = true;
    }
    
    // Update data point list
    updateDataPointList();
    
    // Clear previous series and axes
    chart->removeAllSeries();
    
    // Remove all axes explicitly
    for (QAbstractAxis *axis : chart->axes()) {
        chart->removeAxis(axis);
    }

    // Collect enabled data points
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

    // Create scatter series for measurement data
    auto *scatterSeries = new QScatterSeries();
    scatterSeries->setName("Measured Data");
    scatterSeries->setMarkerSize(12);
    scatterSeries->setColor(QColor(255, 0, 0));
    scatterSeries->setBorderColor(QColor(200, 0, 0));

    // Create line series for fitting
    auto *lineSeries = new QLineSeries();
    lineSeries->setName("Linear Fit");
    lineSeries->setColor(QColor(0, 0, 255));
    lineSeries->setPen(QPen(QBrush(QColor(0, 0, 255)), 2));

    // Add data points
    double minX = enabledSpacings[0], maxX = enabledSpacings[0];
    double minY = enabledResistances[0], maxY = enabledResistances[0];

    for (qsizetype i = 0; i < enabledSpacings.size(); ++i) {
        scatterSeries->append(enabledSpacings[i], enabledResistances[i]);

        // Update range
        if (enabledSpacings[i] < minX) minX = enabledSpacings[i];
        if (enabledSpacings[i] > maxX) maxX = enabledSpacings[i];
        if (enabledResistances[i] < minY) minY = enabledResistances[i];
        if (enabledResistances[i] > maxY) maxY = enabledResistances[i];
    }

    // Add fitted line data points (extend range for better visual effect)
    double extendedMinX = minX - (maxX - minX) * 0.1;
    double extendedMaxX = maxX + (maxX - minX) * 0.1;

    lineSeries->append(extendedMinX, currentSlope * extendedMinX + currentIntercept);
    lineSeries->append(extendedMaxX, currentSlope * extendedMaxX + currentIntercept);

    // Add series to chart
    chart->addSeries(scatterSeries);
    chart->addSeries(lineSeries);

    // Configure axes
    auto *axisX = new QValueAxis();
    auto *axisY = new QValueAxis();

    axisX->setTitleText("Pad Spacing (μm)");
    axisY->setTitleText("Total Resistance (Ω)");
    axisX->setLabelFormat("%.1f");
    axisY->setLabelFormat("%.2f");

    // Set axis range
    axisX->setRange(minX - (maxX - minX) * 0.05, maxX + (maxX - minX) * 0.05);
    axisY->setRange(minY - (maxY - minY) * 0.05, maxY + (maxY - minY) * 0.05);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    scatterSeries->attachAxis(axisX);
    scatterSeries->attachAxis(axisY);
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    // Calculate TLM parameters for display
    double Rsh = currentSlope * 100.0;  // Sheet resistance in Ω/sq
    double Rc = currentIntercept / 20.0; // Contact resistance in Ω·mm
    double Rouc = (Rc * Rc / Rsh) * 1e-2; // Specific contact resistivity in Ω·cm²

    // Add text item to display parameters on chart
    auto* textItem = new QGraphicsSimpleTextItem(chart);
    QString parameterText = QString("Rsh: %1 Ω/sq\nRc: %2 Ω·mm\nρc: %3 Ω·cm²")
                            .arg(Rsh, 0, 'f', 3)
                            .arg(Rc, 0, 'f', 3)
                            .arg(Rouc, 0, 'e', 3);
                            
    textItem->setText(parameterText);
    textItem->setPos(chart->plotArea().left() + 150, chart->plotArea().top() + 10);
    textItem->setBrush(QBrush(Qt::black));
    QFont font = textItem->font();
    font.setPointSize(16);         // Smaller font size
    font.setItalic(true);         // Italic font
    font.setBold(true);           // Bold font
    textItem->setFont(font);
    
    // Add a rectangle background with margin for better visibility
    auto* backgroundRect = new QGraphicsRectItem(chart);
    QRectF textBounds = textItem->boundingRect();
    // Add some padding around the text
    qreal padding = 7.0;
    QRectF rectWithPadding(textBounds.left() - padding, 
                          textBounds.top() - padding,
                          textBounds.width() + 2 * padding, 
                          textBounds.height() + 2 * padding);
    
    backgroundRect->setRect(rectWithPadding);
    backgroundRect->setPos(chart->plotArea().left() + 150, chart->plotArea().top() + 10);
    backgroundRect->setBrush(QBrush(QColor(255, 255, 255, 220))); // More opaque white background
    backgroundRect->setPen(QPen(QColor(50, 50, 50), 2)); // Darker border with thicker line
    backgroundRect->setZValue(-1); // Behind the text

    // Update chart title to include fit information
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
        QString itemText = QString("Point %1: Spacing=%2 μm, Resistance=%3 Ω, Current=%4 A")
                          .arg(i + 1)
                          .arg(originalSpacings[i], 0, 'f', 3)
                          .arg(originalResistances[i], 0, 'f', 3)
                          .arg(originalCurrents[i], 0, 'f', 6);
        
        if (!dataPointEnabled[i]) {
            itemText += " (Removed)";
        }
        
        auto *item = new QListWidgetItem(itemText);
        if (!dataPointEnabled[i]) {
            item->setForeground(QColor(128, 128, 128)); // Gray out removed points
        }
        dataPointList->addItem(item);
    }
}