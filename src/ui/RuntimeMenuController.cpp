#include "RuntimeMenuController.h"
#include "core/MenuPolicy.h"
#include <QGuiApplication>
#include <QScreen>

namespace gpm {
RuntimeMenuController::RuntimeMenuController(ConfigManager* config, PieMenuWidget* pie, QSettings* settings, QObject* parent)
    : QObject(parent), Config(config), Pie(pie), Settings(settings) {
    Hidden = Settings && Settings->value("persistent/hidden", false).toBool();
    connect(Pie, &PieMenuWidget::HidePersistentRequested, this, [this] { SetHidden(true); });
    connect(Pie, &PieMenuWidget::PositionChanged, this, [this](const QPoint& point) { remember("center", point); });
    connect(Pie, &PieMenuWidget::SubmenuChanged, this, [this](const QString& id) { remember("submenu", id); });
    connect(Pie, &PieMenuWidget::MenuClosed, this, [this] { Shortcut.clear(); Current.clear(); refresh(); });
    connect(Config, &ConfigManager::ConfigChanged, this, [this] {
        Shortcut.clear(); Current.clear(); Pie->ResetMenu(); refresh();
    });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, [this] {
        if (Shortcut.isEmpty()) { Current.clear(); refresh(); }
    });
}
void RuntimeMenuController::remember(const QString& key, const QVariant& value) {
    if (Current.isEmpty() || !Shortcut.isEmpty()) return;
    const auto path = "persistent/menus/" + Current + "/" + key;
    Saved.insert(path, value);
    if (Settings) Settings->setValue(path, value);
}
QVariant RuntimeMenuController::recall(const QString& id, const QString& key, const QVariant& fallback) const {
    const auto path = "persistent/menus/" + id + "/" + key;
    if (Saved.contains(path)) return Saved.value(path);
    return Settings ? Settings->value(path, fallback) : fallback;
}
void RuntimeMenuController::SetForegroundProcess(const QString& process) {
    if (Process == process) { refresh(); return; }
    Process = process;
    Shortcut.clear(); Current.clear(); Pie->ResetMenu(); refresh();
}
void RuntimeMenuController::refresh() {
    if (!Shortcut.isEmpty() && !Paused && !Recording) return;
    const auto* profile = PersistentMenu(Config->GetConfig().Profiles, Process);
    const auto id = !Hidden && !Paused && !Recording && profile ? profile->Id : QString();
    if (id == Current && (id.isEmpty() || Pie->IsOpen())) return;
    Current.clear(); Pie->ResetMenu();
    if (id.isEmpty()) return;
    Current = id;
    const auto center = recall(id, "center", QGuiApplication::primaryScreen()->availableGeometry().center()).toPoint();
    Pie->ShowAt(center, *profile, Config->GetConfig().GlobalStyle, true);
    Pie->RestoreSubmenu(recall(id, "submenu").toString());
}
bool RuntimeMenuController::Trigger(const QString& id, const QPoint& point) {
    const auto* profile = Config->FindProfile(id);
    if (Paused || Recording || !profile || !CanTriggerMenu(Config->GetConfig().Profiles, *profile, Process)) return false;
    Shortcut = id; Current = id;
    Pie->ShowAt(point, *profile, Config->GetConfig().GlobalStyle);
    return true;
}
void RuntimeMenuController::CancelShortcut() {
    if (Shortcut.isEmpty()) return;
    Shortcut.clear(); Current.clear(); Pie->ResetMenu(); refresh();
}
void RuntimeMenuController::ConfirmShortcut(const QPoint& point) {
    if (Shortcut.isEmpty()) return;
    Pie->UpdateMousePos(point); Pie->ConfirmSelection();
}
void RuntimeMenuController::SetHidden(bool hidden) {
    if (Hidden == hidden) return;
    Hidden = hidden;
    if (Settings) Settings->setValue("persistent/hidden", hidden);
    refresh(); emit HiddenChanged(hidden);
}
void RuntimeMenuController::SetPaused(bool paused) {
    Paused = paused;
    if (paused) { Shortcut.clear(); Current.clear(); Pie->ResetMenu(); }
    refresh();
}
void RuntimeMenuController::SetRecording(bool recording) {
    Recording = recording;
    if (recording) { Shortcut.clear(); Current.clear(); Pie->ResetMenu(); }
    refresh();
}
}
