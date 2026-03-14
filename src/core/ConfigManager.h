#pragma once

// =============================================================================
// GoPieMenu - ConfigManager
// =============================================================================

#include "IConfigProvider.h"
#include "models/AppConfig.h"

#include <QObject>
#include <QString>
#include <functional>

namespace gpm 
{

class ConfigManager : public QObject, public IConfigProvider 
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject* Parent = nullptr);
    virtual ~ConfigManager() override = default;

    // === IConfigProvider Implementation ===
    [[nodiscard]] virtual AppConfig Load() override;
    virtual bool Save(const AppConfig& InConfig) override;
    [[nodiscard]] virtual QString GetConfigFilePath() const override;

    // === Accessors ===
    [[nodiscard]] const AppConfig& GetConfig() const { return Config; }
    [[nodiscard]] AppConfig& GetConfigMutable() { return Config; }

    // === Profile Management ===
    void AddProfile(PieMenuConfig InProfile);
    void RemoveProfile(const QString& InId);
    [[nodiscard]] PieMenuConfig* FindProfile(const QString& InId);
    [[nodiscard]] const PieMenuConfig* FindProfile(const QString& InId) const;

    // === Lifecycle ===
    void Reload();
    void SaveCurrentConfig();

signals:
    void ConfigChanged();
    void ProfileAdded(const QString& Id);
    void ProfileRemoved(const QString& Id);

private:
    [[nodiscard]] QString GetDefaultConfigDir() const;
    void EnsureConfigDir() const;

    // === Data ===
    AppConfig Config;
    QString   ConfigPath;
};

} // namespace gpm
