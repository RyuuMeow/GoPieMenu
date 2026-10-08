#include "core/ConfigManager.h"
#include "core/HookManager.h"
#include "core/ActionExecutor.h"
#include "ui/PieMenuWidget.h"
#include "ui/SettingsWindow.h"
#include "ui/TrayManager.h"
#include <QApplication>
#include <QQuickStyle>
#include <QSharedMemory>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QIcon>
#include <QTimer>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GoPieMenu");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("GoPieMenu");
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/logo/GoPieMenu.png"));
    QQuickStyle::setStyle("Basic");
    const bool smokeTest = app.arguments().contains("--smoke-test");
    const bool preview = smokeTest || app.arguments().contains("--preview");
    app.setQuitOnLastWindowClosed(preview);
    QSharedMemory instance("GoPieMenu_SingleInstance");
    if (!preview && !instance.create(1)) {
        QMessageBox::information(nullptr, "GoPieMenu", "GoPieMenu is already running.");
        return 0;
    }
    QTemporaryDir previewDir;
    gpm::ConfigManager config(nullptr, preview ? previewDir.filePath("config.json") : QString());
    gpm::ActionExecutor executor;
    executor.RegisterBuiltins();
    gpm::IconService icons;
    gpm::PieMenuWidget pie(&icons);
    gpm::SettingsWindow editor(&config, &icons);
    editor.setPlatformIntegrationEnabled(!preview);
    editor.setPreviewMode(preview);
    if (!editor.window()) return 1;
    gpm::HookManager hooks(&config);
    gpm::TrayManager tray;
    QObject::connect(&hooks, &gpm::HookManager::Triggered, &pie, [&](const QString& id, const QPoint& pos) {
        if (hooks.IsSuspended()) return;
        if (const auto* profile = config.FindProfile(id)) pie.ShowAt(pos, *profile, config.GetConfig().GlobalStyle);
    }, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::MouseMoved, &pie, &gpm::PieMenuWidget::UpdateMousePos, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::TriggerReleased, &pie, [&](const QPoint& pos) {
        pie.UpdateMousePos(pos); pie.ConfirmSelection();
    }, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::CancelRequested, &pie, &gpm::PieMenuWidget::HideMenu, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::WheelScrolled, &pie, &gpm::PieMenuWidget::Scroll, Qt::QueuedConnection);
    QObject::connect(&pie, &gpm::PieMenuWidget::ItemSelected, &executor, [&](int, const gpm::PieItem& item) {
        if (!preview) executor.Execute(item);
    });
    QObject::connect(&executor, &gpm::ActionExecutor::ActionFailed, editor.session(), [&](const QString& name, const QString& error) {
        editor.session()->reportError(name + ": " + error);
    });
    QObject::connect(&editor, &gpm::SettingsWindow::RecordingChanged, &hooks, &gpm::HookManager::SetSuspended);
    QObject::connect(&config, &gpm::ConfigManager::ConfigChanged, &pie, &gpm::PieMenuWidget::HideMenu);
    QObject::connect(&tray, &gpm::TrayManager::SettingsRequested, &editor, &gpm::SettingsWindow::show);
    QObject::connect(&tray, &gpm::TrayManager::PauseToggled, &hooks, [&](bool paused) {
        if (paused) hooks.Uninstall();
        else if (!preview && !hooks.Install()) editor.session()->reportError("Could not install global input hooks.");
    });
    QObject::connect(&tray, &gpm::TrayManager::QuitRequested, &app, [&] {
        if (editor.requestExit()) app.quit();
    });
    if (!preview && !hooks.Install()) editor.session()->reportError("Could not install global input hooks.");
    if (!preview) tray.Show();
    if (preview || app.arguments().contains("--settings") || !QFileInfo::exists(config.GetConfigFilePath()) || !config.LoadError().isEmpty())
        editor.show();
    if (smokeTest) QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    return app.exec();
}
