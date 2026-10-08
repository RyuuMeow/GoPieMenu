// =============================================================================
// GoPieMenu - Tray Manager Implementation
// =============================================================================

#include "TrayManager.h"

#include <QApplication>
#include <QIcon>
#include <QStyle>

static void InitializeBrandResources() { Q_INIT_RESOURCE(resources); }

namespace gpm 
{

TrayManager::TrayManager(QObject* Parent)
    : QObject(Parent)
{
    InitializeBrandResources();
    CreateTrayIcon();
}

TrayManager::~TrayManager()
{
    delete TrayMenu;
}

void TrayManager::CreateTrayIcon()
{
    TrayIcon = new QSystemTrayIcon(this);
    TrayIcon->setIcon(QIcon(QStringLiteral(":/logo/tray-color.ico")));
    TrayIcon->setToolTip(QStringLiteral("GoPieMenu - Active"));

    TrayMenu = new QMenu;
    TrayMenu->setObjectName(QStringLiteral("trayContextMenu"));
    TrayMenu->setAttribute(Qt::WA_TranslucentBackground);
    TrayMenu->setWindowFlag(Qt::FramelessWindowHint);
    TrayMenu->setStyleSheet(QStringLiteral(
        // Match the QML AppMenu/AppMenuItem spacing, colors, and corner radii.
        "QMenu { background: #ffffff; color: #29374b; border: 1px solid #e2e7ef; "
        "        border-radius: 8px; padding: 6px; min-width: 212px; font-family: 'Segoe UI'; font-size: 14px; }"
        "QMenu::item { padding: 10px 12px; min-height: 20px; border-radius: 5px; margin: 0; }"
        "QMenu::item:selected { background: #e2ebf8; color: #29374b; }"
        "QMenu::item:pressed { background: #d2e1f5; }"
        "QMenu::separator { background: #e2e7ef; height: 1px; margin: 6px; }"
        "QMenu::indicator { width: 14px; height: 14px; border: 1px solid #e2e7ef; border-radius: 3px; }"
        "QMenu::indicator:checked { background: #4e80c8; border-color: #4e80c8; }"
    ));

    auto* LocalSettingsAction = TrayMenu->addAction(QStringLiteral("Settings"));
    connect(LocalSettingsAction, &QAction::triggered, this, &TrayManager::SettingsRequested);

    TrayMenu->addSeparator();

    PauseAction = TrayMenu->addAction(QStringLiteral("Pause"));
    PauseAction->setCheckable(true);
    connect(PauseAction, &QAction::toggled, this, [this](bool bChecked) 
    {
        bIsPaused = bChecked;
        PauseAction->setText(bChecked ? QStringLiteral("Resume") : QStringLiteral("Pause"));
        TrayIcon->setToolTip(bChecked ? QStringLiteral("GoPieMenu - Paused")
                                      : QStringLiteral("GoPieMenu - Active"));
        emit PauseToggled(bChecked);
    });

    TrayMenu->addSeparator();

    auto* LocalQuitAction = TrayMenu->addAction(QStringLiteral("Quit"));
    connect(LocalQuitAction, &QAction::triggered, this, &TrayManager::QuitRequested);

    TrayIcon->setContextMenu(TrayMenu);
    connect(TrayIcon, &QSystemTrayIcon::activated, this, &TrayManager::OnTrayActivated);
}

void TrayManager::Show()
{
    TrayIcon->show();
}

void TrayManager::OnTrayActivated(QSystemTrayIcon::ActivationReason Reason)
{
    if (Reason == QSystemTrayIcon::DoubleClick) 
    {
        emit SettingsRequested();
    }
}

} // namespace gpm
