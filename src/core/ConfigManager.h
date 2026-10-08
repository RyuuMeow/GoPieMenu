#pragma once
#include "IConfigProvider.h"
#include <QObject>

namespace gpm {
class ConfigManager : public QObject, public IConfigProvider {
    Q_OBJECT
public:
    explicit ConfigManager(QObject* parent = nullptr, QString path = {});
    [[nodiscard]] ConfigLoadResult Load() override;
    [[nodiscard]] ConfigSaveResult Save(const AppConfig& config) override;
    [[nodiscard]] QString GetConfigFilePath() const override { return ConfigPath; }
    [[nodiscard]] const AppConfig& GetConfig() const { return Config; }
    [[nodiscard]] const PieMenuConfig* FindProfile(const QString& id) const;
    [[nodiscard]] QString LoadError() const { return LastLoadError; }
    [[nodiscard]] ConfigSaveResult Commit(const AppConfig& config);

signals:
    void ConfigChanged();

private:
    QString ConfigPath;
    QString LastLoadError;
    AppConfig Config;
};
}
