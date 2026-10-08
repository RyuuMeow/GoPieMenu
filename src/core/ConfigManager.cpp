#include "ConfigManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <algorithm>

namespace gpm {
ConfigManager::ConfigManager(QObject* parent, QString path) : QObject(parent) {
    ConfigPath = path.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json" : path;
    auto result = Load();
    LastLoadError = result.Error;
    Config = result ? std::move(*result.Value) : AppConfig::CreateDefault();
}
ConfigLoadResult ConfigManager::Load() {
    QFile file(ConfigPath);
    if (!file.exists()) return {AppConfig::CreateDefault(), {}};
    if (!file.open(QIODevice::ReadOnly)) return {{}, file.errorString()};
    const auto data = file.readAll();
    if (file.error() != QFileDevice::NoError) return {{}, file.errorString()};
    return ParseConfig(data);
}
ConfigSaveResult ConfigManager::Save(const AppConfig& config) {
    if (auto error = ValidateConfig(config); !error.isEmpty()) return {false, error};
    if (!QDir().mkpath(QFileInfo(ConfigPath).absolutePath()))
        return {false, QStringLiteral("Cannot create the configuration folder.")};
    QSaveFile file(ConfigPath);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) return {false, file.errorString()};
    const auto bytes = config.Serialize();
    if (file.write(bytes) != bytes.size()) {
        const auto error = file.errorString();
        file.cancelWriting();
        return {false, error};
    }
    if (!file.commit()) return {false, file.errorString()};
    return {true, {}};
}
ConfigSaveResult ConfigManager::Commit(const AppConfig& config) {
    auto result = Save(config);
    if (!result) return result;
    Config = config;
    LastLoadError.clear();
    emit ConfigChanged();
    return result;
}
const PieMenuConfig* ConfigManager::FindProfile(const QString& id) const {
    const auto it = std::ranges::find_if(Config.Profiles, [&](const auto& p) { return p.Id == id; });
    return it == Config.Profiles.end() ? nullptr : &*it;
}
}
