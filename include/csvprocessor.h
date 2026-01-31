#ifndef TLMANALYZER_CSVPARSER_H
#define TLMANALYZER_CSVPARSER_H

#include "datapoint.h"
#include "appconfig.h"

/**
 * @brief Utility class for processing CSV files containing TLM measurement data
 * 
 * The CSVProcessor class provides static methods for parsing CSV files that contain
 * Transmission Line Method (TLM) measurement data. It can process individual files
 * or entire folders of CSV files, extracting spacing information from filenames
 * and electrical measurements from file contents.
 * 
 * The class is designed as a utility with only static methods and no instance data.
 */
class CSVProcessor {
public:
    /**
     * @brief Process all CSV files in a folder
     * @param folderPath Path to the folder containing CSV files
     * @param config Application configuration
     * @return QVector of DataPoint objects extracted from the CSV files
     */
    static QVector<DataPoint> processFolder(const QString &folderPath, const AppConfig& config);
    
    /**
     * @brief Process a single CSV file
     * @param filePath Path to the CSV file to process
     * @param config Application configuration
     * @return DataPoint object containing the extracted measurements
     */
    static DataPoint processFile(const QString &filePath, const AppConfig& config);
    
    /**
     * @brief Extract spacing value from a filename
     * @param filename The filename to parse
     * @return The extracted spacing value, or -1 if no valid spacing was found
     */
    static double extractSpacingFromFilename(const QString &filename);

private:
    /**
     * @brief Private constructor to prevent instantiation
     * 
     * This class is designed as a utility with only static methods, so the 
     * constructor is private to prevent creating instances.
     */
    CSVProcessor() = default; // Static class
};

#endif // TLMANALYZER_CSVPARSER_H