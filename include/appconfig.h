#ifndef TLMANALYZER_APPCONFIG_H
#define TLMANALYZER_APPCONFIG_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>
#include <QVector>

/**
 * @brief Application configuration manager
 * 
 * This class manages application configuration settings, including loading 
 * and saving parameters from/to a JSON file. It combines the configuration
 * data structure with the management functionality.
 */
class AppConfig {
public:
    // Configuration parameters
    double res_voltage;      ///< Resistance voltage parameter
    double channel_length;   ///< Channel length parameter
    
    /**
     * @brief Constructor
     * @param configFile Path to the configuration file
     */
    explicit AppConfig(const QString& configFile = "config.json");
    
    /**
     * @brief Load configuration from file
     * @return True if successful, false otherwise
     */
    bool loadConfig();
    
    /**
     * @brief Save current configuration to file
     * @return True if successful, false otherwise
     */
    bool saveConfig() const;
    
    /**
     * @brief Set resistance voltage parameter and save to config file
     * @param voltage Resistance voltage value
     * @return True if successful, false otherwise
     */
    bool setResistanceVoltage(double voltage);
    
    /**
     * @brief Set channel length parameter and save to config file
     * @param length Channel length value
     * @return True if successful, false otherwise
     */
    bool setChannelLength(double length);
    
    /**
     * @brief Get the configuration file path
     * @return Path to configuration file
     */
    QString getConfigFile() const;

private:
    QString m_configFile;  ///< Path to the configuration file
    
    /**
     * @brief Create a default configuration file
     * @return True if successful, false otherwise
     */
    bool createDefaultConfig() const;
};

// Required for Qt signal/slot mechanism
Q_DECLARE_METATYPE(AppConfig)
Q_DECLARE_METATYPE(QVector<AppConfig>)

#endif // TLMANALYZER_APPCONFIG_H