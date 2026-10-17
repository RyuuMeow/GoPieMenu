#include "core/ConfigManager.h"
#include "core/TranslationManager.h"
#include "core/HookManager.h"
#include "core/ActionExecutor.h"
#include "ui/RuntimeMenuController.h"
#include "core/MenuPolicy.h"
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
#include <QStyleHints>
#include <QSslSocket>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QFont>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GoPieMenu");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("GoPieMenu");
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon(":/logo/GoPieMenu.ico"));
    app.setStyle("Fusion");
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
    QQuickStyle::setStyle("Basic");
    const auto arguments = app.arguments();
#ifdef Q_OS_WIN
    // A restarted child must not contend for the instance lock until its parent exits.
    const int parentArgument = arguments.indexOf("--restart-parent");
    if (parentArgument >= 0) {
        bool valid = false;
        const auto pid = arguments.value(parentArgument + 1).toULong(&valid);
        if (!valid || !pid || pid == GetCurrentProcessId()) return 1;
        HANDLE parent = OpenProcess(SYNCHRONIZE, FALSE, pid);
        if (parent) {
            const auto result = WaitForSingleObject(parent, 30000);
            CloseHandle(parent);
            if (result != WAIT_OBJECT_0) return 1;
        } else if (GetLastError() != ERROR_INVALID_PARAMETER) return 1;
    }
#endif
    const bool smokeTest = app.arguments().contains("--smoke-test");
    if (smokeTest && !QSslSocket::supportsSsl()) return 1;
    const bool preview = smokeTest || app.arguments().contains("--preview");
    app.setQuitOnLastWindowClosed(preview);
    gpm::TranslationManager translations;
    translations.load("system");
    QSharedMemory instance("GoPieMenu_SingleInstance");
    const bool showSettings = preview || !arguments.contains("--autostart") || arguments.contains("--settings");
    const QString activationName = "GoPieMenu_Activation";
    if (!preview && !instance.create(1)) {
        if (showSettings) {
            QLocalSocket activation;
            activation.connectToServer(activationName);
            if (activation.waitForConnected(3000)) {
                activation.write("show\n");
                activation.waitForBytesWritten(1000);
            } else QMessageBox::information(nullptr, "GoPieMenu", QCoreApplication::translate("Application", "GoPieMenu is already running."));
        }
        return 0;
    }
    QLocalServer activationServer;
    if (!preview) {
        QLocalServer::removeServer(activationName);
        activationServer.setSocketOptions(QLocalServer::UserAccessOption);
        activationServer.listen(activationName);
    }
    QTemporaryDir previewDir;
    gpm::ConfigManager config(nullptr, preview ? previewDir.filePath("config.json") : QString());
    translations.load(config.GetConfig().Language);
    app.setFont(QFont(gpm::InterfaceFontFamily(config.GetConfig().Language)));
    gpm::ActionExecutor executor;
    executor.RegisterBuiltins();
    gpm::IconService icons;
    gpm::PieMenuWidget pie(&icons);
    QSettings runtimeSettings;
    gpm::RuntimeMenuController runtime(&config, &pie, preview ? nullptr : &runtimeSettings);
    gpm::UpdateOptions updateOptions;
    updateOptions.language = gpm::ResolveLanguage(config.GetConfig().Language);
    std::unique_ptr<gpm::UpdateManager> updates;
    if (!preview) updates = std::make_unique<gpm::UpdateManager>(updateOptions);
    gpm::SettingsWindow editor(&config, &icons, nullptr, updates.get());
    editor.setPlatformIntegrationEnabled(!preview);
    editor.setPreviewMode(preview);
    editor.setRestartHandler([preview] {
        QStringList restartArguments{"--settings"};
        if (preview) restartArguments << "--preview";
#ifdef Q_OS_WIN
        restartArguments << "--restart-parent" << QString::number(QCoreApplication::applicationPid());
#endif
        return QProcess::startDetached(QCoreApplication::applicationFilePath(), restartArguments);
    });
    if (!preview) editor.syncAutoStart();
    QObject::connect(&activationServer, &QLocalServer::newConnection, &editor, [&] {
        while (auto* socket = activationServer.nextPendingConnection()) {
            socket->setParent(&activationServer);
            auto receive = [socket, &editor] {
                if (!socket->canReadLine()) return;
                if (socket->readLine().trimmed() == "show") editor.show();
                socket->disconnectFromServer();
            };
            QObject::connect(socket, &QLocalSocket::readyRead, &editor, receive);
            QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            receive();
        }
    });
    if (!editor.window()) return 1;
    gpm::HookManager hooks(&config);
    gpm::TrayManager tray;
    tray.SetPersistentHidden(runtime.IsHidden());
    QObject::connect(&tray, &gpm::TrayManager::PersistentHiddenToggled, &runtime, &gpm::RuntimeMenuController::SetHidden);
    QObject::connect(&runtime, &gpm::RuntimeMenuController::HiddenChanged, &tray, &gpm::TrayManager::SetPersistentHidden);
    QObject::connect(&hooks, &gpm::HookManager::ForegroundChanged, &runtime, &gpm::RuntimeMenuController::SetForegroundProcess);
    QObject::connect(&hooks, &gpm::HookManager::PointerPressed, &pie, &gpm::PieMenuWidget::DismissContextOutside);
    QObject::connect(&editor, &gpm::SettingsWindow::updateToastRequested, &tray, &gpm::TrayManager::ShowUpdateNotification);
    QObject::connect(&tray, &gpm::TrayManager::UpdateRequested, &editor, &gpm::SettingsWindow::showUpdate);
    if (updates) QObject::connect(updates.get(), &gpm::UpdateManager::changed, &tray, [&] {
        tray.SetUpdateAvailable(updates->available() ? updates->version() : QString());
    });
    QObject::connect(&hooks, &gpm::HookManager::Triggered, &pie, [&](const QString& id, const QPoint& pos) {
        if (hooks.IsSuspended()) return;
        runtime.SetForegroundProcess(hooks.foregroundProcess());
        if (!runtime.Trigger(id, pos)) hooks.Cancel();
    }, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::MouseMoved, &pie, &gpm::PieMenuWidget::UpdateMousePos, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::TriggerReleased, &pie, [&](const QPoint& pos) {
        runtime.ConfirmShortcut(pos);
    }, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::CancelRequested, &runtime, &gpm::RuntimeMenuController::CancelShortcut, Qt::QueuedConnection);
    QObject::connect(&hooks, &gpm::HookManager::WheelScrolled, &pie, &gpm::PieMenuWidget::Scroll, Qt::QueuedConnection);
    QObject::connect(&pie, &gpm::PieMenuWidget::ItemSelected, &executor, [&](int, const gpm::PieItem& item) {
        if (pie.IsPersistent()) {
            const auto* eligible = gpm::PersistentMenu(config.GetConfig().Profiles, hooks.foregroundProcess());
            if (!eligible || eligible->Id != runtime.CurrentProfile()) return;
        }
        if (!preview) executor.Execute(item);
    });
    QObject::connect(&executor, &gpm::ActionExecutor::ActionFailed, editor.session(), [&](const QString& name, const QString& error) {
        editor.session()->reportError(name + ": " + error);
    });
    QObject::connect(&editor, &gpm::SettingsWindow::RecordingChanged, &hooks, &gpm::HookManager::SetSuspended);
    QObject::connect(&editor, &gpm::SettingsWindow::RecordingChanged, &runtime, &gpm::RuntimeMenuController::SetRecording);
    QObject::connect(&tray, &gpm::TrayManager::SettingsRequested, &editor, &gpm::SettingsWindow::show);
    QObject::connect(&tray, &gpm::TrayManager::PauseToggled, &hooks, [&](bool paused) {
        runtime.SetPaused(paused);
        if (paused) hooks.Uninstall();
        else if (!preview && !hooks.Install()) editor.session()->reportError(QCoreApplication::translate("Application", "Could not install global input hooks."));
    });
    QObject::connect(&tray, &gpm::TrayManager::QuitRequested, &editor, &gpm::SettingsWindow::requestExit);
    QObject::connect(&editor, &gpm::SettingsWindow::exitConfirmed, &app, &QCoreApplication::quit);
    if (!preview && !hooks.Install()) editor.session()->reportError(QCoreApplication::translate("Application", "Could not install global input hooks."));
    if (!preview) tray.Show();
    if (showSettings || !config.LoadError().isEmpty())
        editor.show();
    if (smokeTest) QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    if (updates) QTimer::singleShot(0, updates.get(), [&] { updates->check(); });
    return app.exec();
}
