// =============================================================================
// GoPieMenu - Application Entry Point
// =============================================================================

#include "core/ConfigManager.h"
#include "core/HookManager.h"
#include "core/ActionExecutor.h"
#include "ui/PieMenuWidget.h"
#include "ui/SettingsWindow.h"
#include "ui/TrayManager.h"

#include <QApplication>
#include <QIcon>
#include <QSharedMemory>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication App(argc, argv);
    App.setApplicationName(QStringLiteral("GoPieMenu"));
    App.setApplicationVersion(QStringLiteral(APP_VERSION));
    App.setOrganizationName(QStringLiteral("GoPieMenu"));
    App.setQuitOnLastWindowClosed(false);
    
    // === Application Icon ===
    App.setWindowIcon(QIcon(QStringLiteral(":/logo/GoPieMenu.png")));

    // === Single Instance Check ===
    QSharedMemory SingleInstance(QStringLiteral("GoPieMenu_SingleInstance"));
    if (!SingleInstance.create(1)) 
    {
        // Handle Windows crash scenario where memory was not released
        if (SingleInstance.attach()) 
        {
            SingleInstance.detach();
        }
        
        if (!SingleInstance.create(1)) 
        {
            QMessageBox::information(nullptr, QStringLiteral("GoPieMenu"),
                                      QStringLiteral("GoPieMenu is already running."));
            return 0;
        }
    }

    // === Core Components ===
    gpm::ConfigManager ConfigMgr;
    gpm::ActionExecutor Executor;
    Executor.RegisterBuiltins();
    gpm::HookManager HookMgr(&ConfigMgr);

    // === UI Components ===
    gpm::PieMenuWidget PieWidget;
    gpm::SettingsWindow SettingsWindowObj(&ConfigMgr, &Executor);
    gpm::TrayManager TrayMgr;
    TrayMgr.SetSettingsWindow(&SettingsWindowObj);

    // === Connections ===
    // CRITICAL: Use QueuedConnection for hook signals so they execute
    // after the hook callback returns (not inside WH_*_LL proc)

    QObject::connect(&HookMgr, &gpm::HookManager::Triggered,
        &PieWidget, [&](const QString& ProfileId, const QPoint& MousePos) 
        {
            if (auto* Profile = ConfigMgr.FindProfile(ProfileId)) 
            {
                PieWidget.ShowAt(MousePos, *Profile, ConfigMgr.GetConfig().GlobalStyle);
            }
        }, Qt::QueuedConnection);

    QObject::connect(&HookMgr, &gpm::HookManager::MouseMoved,
        &PieWidget, &gpm::PieMenuWidget::UpdateMousePos,
        Qt::QueuedConnection);

    QObject::connect(&HookMgr, &gpm::HookManager::TriggerReleased,
        &PieWidget, [&](const QPoint&) 
        {
            PieWidget.ConfirmSelection();
        }, Qt::QueuedConnection);

    QObject::connect(&HookMgr, &gpm::HookManager::CancelRequested,
        &PieWidget, &gpm::PieMenuWidget::HideMenu,
        Qt::QueuedConnection);

    // Pie Menu -> Action Executor
    QObject::connect(&PieWidget, &gpm::PieMenuWidget::ItemSelected,
        [&](int, const gpm::PieItem& Item) 
        {
            Executor.Execute(Item);
        });

    // Tray
    QObject::connect(&TrayMgr, &gpm::TrayManager::SettingsRequested,
        [&]() 
        {
            SettingsWindowObj.show();
            SettingsWindowObj.raise();
            SettingsWindowObj.activateWindow();
        });

    QObject::connect(&TrayMgr, &gpm::TrayManager::PauseToggled,
        [&](bool bPaused) 
        {
            if (bPaused) 
            {
                HookMgr.Uninstall();
            }
            else 
            {
                HookMgr.Install();
            }
        });

    QObject::connect(&TrayMgr, &gpm::TrayManager::QuitRequested,
        &App, &QApplication::quit);

    QObject::connect(&SettingsWindowObj, &gpm::SettingsWindow::ConfigUpdated,
        [&]() 
        {
            if (HookMgr.IsInstalled()) 
            {
                HookMgr.Uninstall();
                HookMgr.Install();
            }
        });

    // === Execution ===
    if (!HookMgr.Install()) 
    {
        qWarning() << "[Main] Failed to install hooks. Try running as Administrator.";
    }

    TrayMgr.Show();
    qDebug() << "[Main] GoPieMenu started. Profiles:" << ConfigMgr.GetConfig().Profiles.size();

    return App.exec();
}
