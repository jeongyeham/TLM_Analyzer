#include "include/appconfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDateTime>

/**
 * @brief Constructor
 * @param configFile Path to the configuration file
 */
AppConfig::AppConfig(const QString& configFile)
    : res_voltage(1.0), channel_length(100.0), m_configFile(configFile)
{
    // Determine the platform-appropriate config directory
    QString standardConfigDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (standardConfigDir.isEmpty()) {
        // Fallback to AppDataLocation
        standardConfigDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    if (!standardConfigDir.isEmpty()) {
        QDir dir(standardConfigDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        QString targetPath = dir.filePath(QFileInfo(m_configFile).fileName());

        // If the provided config file is not the same as target and exists, try to migrate
        if (QFileInfo(m_configFile).absoluteFilePath() != QFileInfo(targetPath).absoluteFilePath()
            && QFile::exists(m_configFile)) {
            // Backup target if exists
            if (QFile::exists(targetPath)) {
                QString backupName = targetPath + QStringLiteral(".backup.") + QDateTime::currentDateTime().toString(Qt::ISODate);
                if (!QFile::copy(targetPath, backupName)) {
                    qDebug() << "Failed to backup existing target config to" << backupName;
                }
            }

            // Try to copy provided config to target location
            if (QFile::copy(m_configFile, targetPath)) {
                qDebug() << "Migrated config file to standard location:" << targetPath;
                // After copying, attempt to load from the new location; if successful, set m_configFile to target
                QString original = m_configFile;
                m_configFile = targetPath;
                if (!loadConfig()) {
                    // Rollback: restore original path and try to restore backup if present
                    qDebug() << "Migration failed while loading new config; rolling back to original.";
                    m_configFile = original;
                    // Attempt to restore from backup if it exists (best-effort)
                    // (No hard failure; we keep using original if available)
                }
            } else {
                qDebug() << "Failed to migrate config file to" << targetPath << "; will continue using" << m_configFile;
            }
        } else if (QFile::exists(targetPath)) {
            // Use the standard location config if it exists
            m_configFile = targetPath;
        } else {
            // If neither exists, create default at the standard location
            m_configFile = targetPath;
            createDefaultConfig();
            loadConfig();
        }
    } else {
        // Could not determine a standard config location; fallback to provided path
        qDebug() << "Could not determine standard config dir; using provided config path:" << m_configFile;
        if (!loadConfig()) {
            createDefaultConfig();
            loadConfig();
        }
    }
}

/**
 * @brief Load configuration from file
 * @return True if successful, false otherwise
 */
bool AppConfig::loadConfig()
{
    QFile file(m_configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open config file for reading:" << m_configFile;
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error in config file:" << parseError.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format in config file";
        return false;
    }
    
    QJsonObject obj = doc.object();
    
    // Load configuration values
    if (obj.contains("res_voltage") && obj["res_voltage"].isDouble()) {
        res_voltage = obj["res_voltage"].toDouble();
    }
    
    if (obj.contains("channel_length") && obj["channel_length"].isDouble()) {
        channel_length = obj["channel_length"].toDouble();
    }
    
    return true;
}

/**
 * @brief Save current configuration to file
 * @return True if successful, false otherwise
 */
bool AppConfig::saveConfig() const
{
    QJsonObject obj;
    
    // Save configuration values
    obj["res_voltage"] = res_voltage;
    obj["channel_length"] = channel_length;
    
    QJsonDocument doc(obj);
    
    QFile file(m_configFile);
    QDir dir = QFileInfo(m_configFile).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open config file for writing:" << m_configFile;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

/**
 * @brief Set resistance voltage parameter and save to config file
 * @param voltage Resistance voltage value
 * @return True if successful, false otherwise
 */
bool AppConfig::setResistanceVoltage(double voltage)
{
    res_voltage = voltage;
    return saveConfig();
}

/**
 * @brief Set channel length parameter and save to config file
 * @param length Channel length value
 * @return True if successful, false otherwise
 */
bool AppConfig::setChannelLength(double length)
{
    channel_length = length;
    return saveConfig();
}

/**
 * @brief Get the configuration file path
 * @return Path to configuration file
 */
QString AppConfig::getConfigFile() const
{
    return m_configFile;
}

/**
 * @brief Create a default configuration file
 * @return True if successful, false otherwise
 */
bool AppConfig::createDefaultConfig() const
{
    QJsonObject obj;
    
    // Save default values
    obj["res_voltage"] = res_voltage;
    obj["channel_length"] = channel_length;
    
    QJsonDocument doc(obj);
    
    QFile file(m_configFile);
    QDir dir = QFileInfo(m_configFile).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not create default config file:" << m_configFile;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

