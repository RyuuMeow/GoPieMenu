// =============================================================================
// GoPieMenu - Tray Manager Implementation
// =============================================================================

#include "TrayManager.h"
#include "SettingsWindow.h"

#include <QApplication>
#include <QIcon>
#include <QStyle>

namespace gpm 
{

TrayManager::TrayManager(QObject* Parent)
    : QObject(Parent)
{
    CreateTrayIcon();
}

TrayManager::~TrayManager()
{
    delete TrayMenu;
}

void TrayManager::CreateTrayIcon()
{
    TrayIcon = new QSystemTrayIcon(this);
    TrayIcon->setIcon(QIcon(QStringLiteral(":/logo/GoPieMenu.png")));
    TrayIcon->setToolTip(QStringLiteral("GoPieMenu - Active"));

    TrayMenu = new QMenu;
    TrayMenu->setStyleSheet(QStringLiteral(
        "QMenu { background: #16161e; color: #c0c0d8; border: 1px solid #2a2a40; "
        "        border-radius: 6px; padding: 4px; font-family: 'Segoe UI'; font-size: 12px; }"
        "QMenu::item { padding: 6px 24px 6px 16px; border-radius: 4px; margin: 2px 4px; }"
        "QMenu::item:selected { background: #2a2a50; color: #e0e0ff; }"
        "QMenu::separator { background: #2a2a40; height: 1px; margin: 4px 12px; }"
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
