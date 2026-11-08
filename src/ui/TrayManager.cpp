// =============================================================================
// GoPieMenu - Tray Manager Implementation
// =============================================================================

#include "TrayManager.h"

#include <QApplication>
#include <QIcon>
#include <QStyle>
#include <QSignalBlocker>

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
#ifdef Q_OS_MACOS
    // The menu bar draws a monochrome template image and tints it for light and
    // dark appearances; Qt applies QIcon::isMask as NSImage.isTemplate.
    QIcon trayIcon(QStringLiteral(":/logo/trayTemplate.png"));
    if (trayIcon.isNull()) trayIcon = QIcon(QStringLiteral(":/logo/GoPieMenu.png"));
    else trayIcon.setIsMask(true);
    TrayIcon->setIcon(trayIcon);
#else
    TrayIcon->setIcon(QIcon(QStringLiteral(":/logo/tray-color.ico")));
#endif
    TrayIcon->setToolTip(tr("GoPieMenu - Active"));

    TrayMenu = new QMenu;
    TrayMenu->setObjectName(QStringLiteral("trayContextMenu"));
    TrayMenu->setAttribute(Qt::WA_TranslucentBackground);
    TrayMenu->setWindowFlag(Qt::FramelessWindowHint);
    TrayMenu->setStyleSheet(QStringLiteral(
        // Match the QML AppMenu/AppMenuItem spacing, colors, and corner radii.
        "QMenu { background: #ffffff; color: #29374b; border: 1px solid #e2e7ef; "
        "        border-radius: 8px; padding: 6px; min-width: 212px; font-family: '%1'; font-size: 14px; }"
        "QMenu::item { padding: 10px 12px; min-height: 20px; border-radius: 5px; margin: 0; }"
        "QMenu::item:selected { background: #e2ebf8; color: #29374b; }"
        "QMenu::item:pressed { background: #d2e1f5; }"
        "QMenu::separator { background: #e2e7ef; height: 1px; margin: 6px; }"
        "QMenu::indicator { width: 14px; height: 14px; border: 1px solid #e2e7ef; border-radius: 3px; }"
        "QMenu::indicator:checked { background: #4e80c8; border-color: #4e80c8; }"
    ).arg(QApplication::font().family()));

    auto* LocalSettingsAction = TrayMenu->addAction(tr("Settings"));
    connect(LocalSettingsAction, &QAction::triggered, this, &TrayManager::SettingsRequested);

    UpdateAction = TrayMenu->addAction(QString());
    UpdateAction->setObjectName("trayUpdateAction");
    UpdateAction->setVisible(false);
    connect(UpdateAction, &QAction::triggered, this, &TrayManager::UpdateRequested);
    connect(TrayIcon, &QSystemTrayIcon::messageClicked, this, [this] {
        if (UpdateAction->isVisible()) emit UpdateRequested();
    });

    TrayMenu->addSeparator();

    PersistentAction = TrayMenu->addAction(tr("Hide all persistent menus"));
    PersistentAction->setObjectName("trayPersistentAction");
    PersistentAction->setCheckable(true);
    connect(PersistentAction, &QAction::toggled, this, &TrayManager::PersistentHiddenToggled);

    PauseAction = TrayMenu->addAction(tr("Pause"));
    PauseAction->setObjectName("trayPauseAction");
    PauseAction->setCheckable(true);
    connect(PauseAction, &QAction::toggled, this, [this](bool bChecked) 
    {
        bIsPaused = bChecked;
        PauseAction->setText(bChecked ? tr("Resume") : tr("Pause"));
        TrayIcon->setToolTip(bChecked ? tr("GoPieMenu - Paused")
                                      : tr("GoPieMenu - Active"));
        emit PauseToggled(bChecked);
    });

    TrayMenu->addSeparator();

    auto* LocalQuitAction = TrayMenu->addAction(tr("Quit"));
    connect(LocalQuitAction, &QAction::triggered, this, &TrayManager::QuitRequested);

    TrayIcon->setContextMenu(TrayMenu);
    connect(TrayIcon, &QSystemTrayIcon::activated, this, &TrayManager::OnTrayActivated);
}

void TrayManager::Show()
{
    TrayIcon->show();
}
void TrayManager::SetPersistentHidden(bool hidden) {
    const QSignalBlocker blocker(PersistentAction);
    PersistentAction->setChecked(hidden);
}

void TrayManager::SetUpdateAvailable(const QString& version) {
    UpdateAction->setText(tr("Update available: %1").arg(version));
    UpdateAction->setVisible(!version.isEmpty());
}

void TrayManager::ShowUpdateNotification(const QString& version) {
    SetUpdateAvailable(version);
    TrayIcon->showMessage(tr("Update available"),
        tr("GoPieMenu %1 is available. Click to view what's new.").arg(version),
        QSystemTrayIcon::Information, 10000);
}

void TrayManager::OnTrayActivated(QSystemTrayIcon::ActivationReason Reason)
{
#ifdef Q_OS_MACOS
    // macOS opens the context menu on a single click and never reports a
    // double click while a menu is attached; Settings stays one click away in
    // that menu, so no additional activation handling is needed.
    Q_UNUSED(Reason);
#else
    if (Reason == QSystemTrayIcon::DoubleClick) 
    {
        emit SettingsRequested();
    }
#endif
}

} // namespace gpm
