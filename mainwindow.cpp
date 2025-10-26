//
// Created by jeong on 2025/10/25.
//
#include "mainwindow.h"
#include "include/datamanager.h"
#include "include/csvprocessor.h"
#include "include/calculator.h"
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
    , dataManager(new DataManager(this))
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
    // Allow negative values for voltage
    doubleValidator->setBottom(-1000000); // Set a reasonable lower limit
    doubleValidator->setTop(1000000);     // Set a reasonable upper limit

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
    clearDisabledPointsButton = new QPushButton("Clear Removed Points");
    removePointButton->setEnabled(false);
    dataPointButtonLayout->addWidget(addPointButton);
    dataPointButtonLayout->addWidget(removePointButton);
    dataPointButtonLayout->addWidget(clearDisabledPointsButton);
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
    connect(clearDisabledPointsButton, &QPushButton::clicked, this, &MainWindow::clearDisabledPoints);
    connect(dataPointList, &QListWidget::itemSelectionChanged, this, &MainWindow::onDataPointSelectionChanged);

    // Connect data manager signals
    connect(dataManager, &DataManager::dataChanged, this, &MainWindow::onDataChanged);
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
    if (!ok || qFuzzyIsNull(voltage)) {
        QMessageBox::warning(this, "Warning", "Please enter a valid non-zero voltage.");
        return;
    }

    progressBar->setVisible(true);
    progressBar->setValue(0);
    analyzeButton->setEnabled(false);

    // Process CSV files using CSVProcessor
    QVector<DataPoint> dataPoints = CSVProcessor::processFolder(currentFolder, voltage);
    progressBar->setValue(30); // 30% progress after folder processing
    
    // Clear existing data and add new data points
    dataManager->clearDataPoints();
    for (int i = 0; i < dataPoints.size(); ++i) {
        dataManager->addDataPoint(dataPoints.at(i));
        // Update progress based on data points processed
        int progress = 30 + (i * 40 / dataPoints.size());
        progressBar->setValue(progress);
    }
    
    // Update result text with detailed information
    QString result = QString("Analysis complete.\n\n"
                            "Data Points Processed: %1\n"
                            "Applied Voltage: %2 V\n")
                            .arg(dataPoints.size())
                            .arg(voltage, 0, 'f', 3);
    resultText->setText(result);
    progressBar->setValue(80); // 80% progress after data setup
    
    // Manually trigger data changed to update the analysis results
    onDataChanged();
    
    // Enable analyze button
    analyzeButton->setEnabled(true);
    progressBar->setValue(100); // 100% progress when complete
    
    // Hide progress bar after a delay, but preserve any text that might be added later
    QTimer::singleShot(2000, this, [this]() {
        progressBar->setVisible(false);
        progressBar->setValue(0);
    });
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
    if (!voltageOk || qFuzzyIsNull(voltage)) {
        QMessageBox::warning(this, "Invalid Voltage", "Please enter a valid non-zero resistance voltage value.");
        return;
    }
    
    // Check if current is 0 to avoid division by 0
    if (qFuzzyIsNull(current)) {
        QMessageBox::warning(this, "Invalid Input", "Current value cannot be zero.");
        return;
    }
    
    // Add data point through DataManager
    dataManager->addManualDataPoint(spacing, current, voltage);
}

void MainWindow::removeDataPoint() const {
    int selectedIndex = dataPointList->currentRow();
    if (selectedIndex >= 0 && selectedIndex < dataManager->size()) {
        // Disable selected data point
        dataManager->setDataPointEnabled(selectedIndex, false);
    }
}

void MainWindow::clearDisabledPoints() const {
    // Clear all disabled data points
    dataManager->clearDisabledDataPoints();
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

void MainWindow::onPlotDataReady(const QVector<double> &spacings,
                               const QVector<double> &resistances,
                               const QVector<double> &currents,
                               double slope, double intercept)
{
    // Clear previous series and axes
    chart->removeAllSeries();
    
    // Remove all axes explicitly
    for (QAbstractAxis *axis : chart->axes()) {
        chart->removeAxis(axis);
    }
    
    // Remove any existing parameter text items and background rectangles
    if (textItem) {
        delete textItem;
        textItem = nullptr;
    }
    
    if (backgroundRect) {
        delete backgroundRect;
        backgroundRect = nullptr;
    }

    if (spacings.isEmpty()) {
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
    double minX = spacings[0], maxX = spacings[0];
    double minY = resistances[0], maxY = resistances[0];

    for (qsizetype i = 0; i < spacings.size(); ++i) {
        scatterSeries->append(spacings[i], resistances[i]);

        // Update range
        if (spacings[i] < minX) minX = spacings[i];
        if (spacings[i] > maxX) maxX = spacings[i];
        if (resistances[i] < minY) minY = resistances[i];
        if (resistances[i] > maxY) maxY = resistances[i];
    }

    // Add fitted line data points (extend range for better visual effect)
    double extendedMinX = minX - (maxX - minX) * 0.1;
    double extendedMaxX = maxX + (maxX - minX) * 0.1;

    lineSeries->append(extendedMinX, slope * extendedMinX + intercept);
    lineSeries->append(extendedMaxX, slope * extendedMaxX + intercept);

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
    double Rsh = slope * 100.0;  // Sheet resistance in Ω/sq
    double Rc = intercept / 20.0; // Contact resistance in Ω·mm
    double Rouc = (Rc * Rc / Rsh) * 1e-2; // Specific contact resistivity in Ω·cm²

    // Add text item to display parameters on chart
    textItem = new QGraphicsSimpleTextItem(chart);
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
    backgroundRect = new QGraphicsRectItem(chart);
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
                   .arg(slope, 0, 'f', 4)
                   .arg(intercept, 0, 'f', 4));
}

void MainWindow::onDataPointSelectionChanged() const {
    removePointButton->setEnabled(!dataPointList->selectedItems().isEmpty());
}

void MainWindow::updateDataPointList() const {
    dataPointList->clear();
    
    const QVector<DataPoint>& dataPoints = dataManager->getDataPoints();
    for (int i = 0; i < dataPoints.size(); ++i) {
        const DataPoint& point = dataPoints.at(i);
        QString itemText = QString("Point %1: Spacing=%2 μm, Resistance=%3 Ω, Current=%4 A")
                          .arg(i + 1)
                          .arg(point.spacing, 0, 'f', 3)
                          .arg(point.resistance, 0, 'f', 3)
                          .arg(point.current, 0, 'f', 6);
        
        if (!point.enabled) {
            itemText += " (Removed)";
        }
        
        auto *item = new QListWidgetItem(itemText);
        if (!point.enabled) {
            item->setForeground(QColor(128, 128, 128)); // Gray out removed points
        }
        dataPointList->addItem(item);
    }
}

void MainWindow::onDataChanged()
{
    updateDataPointList();
    
    // Get enabled data points for plotting
    QVector<DataPoint> enabledPoints = dataManager->getEnabledDataPoints();
    
    if (enabledPoints.size() < 2) {
        // Not enough points for regression
        if (!enabledPoints.isEmpty()) {
            // With only 1 point, we can't calculate a meaningful regression line
            // Just show the point without the line
            const DataPoint& point = enabledPoints.first();
            QVector<double> spacings = {point.spacing};
            QVector<double> resistances = {point.resistance};
            QVector<double> currents = {point.current};
            onPlotDataReady(spacings, resistances, currents, 0.0, point.resistance);
            
            // Update result text with single point information
            QString singlePointResult = resultText->toPlainText();
            singlePointResult += QString("\nInsufficient data points for TLM analysis.\n"
                                       "At least 2 points are required for linear regression.\n"
                                       "Currently showing single point: %1 μm, %2 Ω")
                                       .arg(point.spacing, 0, 'f', 3)
                                       .arg(point.resistance, 0, 'f', 3);
            resultText->setText(singlePointResult);
        } else {
            // No points left
            chart->removeAllSeries();
            for (QAbstractAxis *axis : chart->axes()) {
                chart->removeAxis(axis);
            }
            chart->setTitle("TLM Analysis - Resistance vs Pad Spacing");
            resultText->setText("No data points available for analysis.");
        }
        return;
    }
    
    // Perform linear regression on enabled points
    Calculator::TLMResult result;
    if (dataManager->calculateTLMResults(result)) {
        // Prepare data for plotting
        QVector<double> spacings, resistances, currents;
        for (const DataPoint& point : enabledPoints) {
            spacings.append(point.spacing);
            resistances.append(point.resistance);
            currents.append(point.current);
        }
        
        onPlotDataReady(spacings, resistances, currents, result.slope, result.intercept);
        
        // Update result text with detailed TLM analysis results
        QString detailedResult = resultText->toPlainText();
        detailedResult += QString("\nTLM Analysis Results:\n"
                                 "=====================\n"
                                 "Slope: %1 Ω/μm\n"
                                 "Intercept: %2 Ω\n"
                                 "Sheet Resistance (Rsh): %3 Ω/sq\n"
                                 "Contact Resistance (Rc): %4 Ω·mm\n"
                                 "Specific Contact Resistivity (ρc): %5 Ω·cm²\n")
                                 .arg(result.slope, 0, 'e', 3)
                                 .arg(result.intercept, 0, 'f', 3)
                                 .arg(result.sheetResistance, 0, 'f', 3)
                                 .arg(result.contactResistance, 0, 'f', 3)
                                 .arg(result.specificContactResistivity, 0, 'e', 3);
        resultText->setText(detailedResult);
    }
}