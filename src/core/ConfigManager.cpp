// =============================================================================
// GoPieMenu - ConfigManager Implementation
// =============================================================================

#include "ConfigManager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>

namespace gpm 
{

ConfigManager::ConfigManager(QObject* Parent)
    : QObject(Parent)
{
    ConfigPath = GetConfigFilePath();
    Reload();
}

AppConfig ConfigManager::Load()
{
    QFile ConfigFile(ConfigPath);
    if (!ConfigFile.exists()) 
    {
        qDebug() << "[ConfigManager] No config file found, using defaults.";
        return AppConfig::CreateDefault();
    }

    if (!ConfigFile.open(QIODevice::ReadOnly)) 
    {
        qWarning() << "[ConfigManager] Failed to open config:" << ConfigPath;
        return AppConfig::CreateDefault();
    }

    auto Data = ConfigFile.readAll();
    ConfigFile.close();

    auto NewConfig = AppConfig::Deserialize(Data);
    qDebug() << "[ConfigManager] Loaded" << NewConfig.Profiles.size() << "profiles.";
    return NewConfig;
}

bool ConfigManager::Save(const AppConfig& InConfig)
{
    EnsureConfigDir();

    QFile ConfigFile(ConfigPath);
    if (!ConfigFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
    {
        qWarning() << "[ConfigManager] Failed to write config:" << ConfigPath;
        return false;
    }

    ConfigFile.write(InConfig.Serialize());
    ConfigFile.close();

    qDebug() << "[ConfigManager] Config saved.";
    return true;
}

QString ConfigManager::GetConfigFilePath() const
{
    return GetDefaultConfigDir() + QStringLiteral("/config.json");
}

void ConfigManager::AddProfile(PieMenuConfig InProfile)
{
    auto NewId = InProfile.Id;
    Config.Profiles.push_back(std::move(InProfile));
    SaveCurrentConfig();
    emit ProfileAdded(NewId);
    emit ConfigChanged();
}

void ConfigManager::RemoveProfile(const QString& InId)
{
    auto It = std::ranges::find_if(Config.Profiles,
        [&](const auto& P) { return P.Id == InId; });

    if (It != Config.Profiles.end()) 
    {
        Config.Profiles.erase(It);
        SaveCurrentConfig();
        emit ProfileRemoved(InId);
        emit ConfigChanged();
    }
}

PieMenuConfig* ConfigManager::FindProfile(const QString& InId)
{
    auto It = std::ranges::find_if(Config.Profiles,
        [&](const auto& P) { return P.Id == InId; });
    return It != Config.Profiles.end() ? &(*It) : nullptr;
}

const PieMenuConfig* ConfigManager::FindProfile(const QString& InId) const
{
    auto It = std::ranges::find_if(Config.Profiles,
        [&](const auto& P) { return P.Id == InId; });
    return It != Config.Profiles.end() ? &(*It) : nullptr;
}

void ConfigManager::Reload()
{
    Config = Load();
    emit ConfigChanged();
}

void ConfigManager::SaveCurrentConfig()
{
    Save(Config);
}

QString ConfigManager::GetDefaultConfigDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

void ConfigManager::EnsureConfigDir() const
{
    QDir Dir(GetDefaultConfigDir());
    if (!Dir.exists()) 
    {
        Dir.mkpath(".");
    }
}

} // namespace gpm
