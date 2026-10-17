#pragma once
#include "core/ConfigManager.h"
#include "PieMenuWidget.h"
#include <QSettings>

namespace gpm {
// One window serves both display modes. Foreground selection precedes visibility.
class RuntimeMenuController : public QObject {
    Q_OBJECT
public:
    RuntimeMenuController(ConfigManager* config, PieMenuWidget* pie, QSettings* settings = nullptr, QObject* parent = nullptr);
    void SetForegroundProcess(const QString& process);
    bool Trigger(const QString& id, const QPoint& point);
    void CancelShortcut();
    void ConfirmShortcut(const QPoint& point);
    void SetHidden(bool hidden);
    void SetPaused(bool paused);
    void SetRecording(bool recording);
    bool IsHidden() const { return Hidden; }
    QString CurrentProfile() const { return Current; }
signals:
    void HiddenChanged(bool hidden);
private:
    void refresh();
    void remember(const QString& key, const QVariant& value);
    QVariant recall(const QString& id, const QString& key, const QVariant& fallback = {}) const;
    ConfigManager* Config;
    PieMenuWidget* Pie;
    QSettings* Settings;
    QHash<QString, QVariant> Saved;
    QString Process, Current, Shortcut;
    bool Hidden = false, Paused = false, Recording = false;
};
}
