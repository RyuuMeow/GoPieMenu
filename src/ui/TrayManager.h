#pragma once

// =============================================================================
// GoPieMenu - System Tray Manager
// =============================================================================

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

namespace gpm 
{

class TrayManager : public QObject 
{
    Q_OBJECT

public:
    explicit TrayManager(QObject* Parent = nullptr);
    virtual ~TrayManager() override;

    /** Makes the tray icon visible */
    void Show();
    void SetUpdateAvailable(const QString& version);
    void ShowUpdateNotification(const QString& version);
    void SetPersistentHidden(bool hidden);
    
signals:
    void SettingsRequested();
    void PauseToggled(bool bPaused);
    void QuitRequested();
    void UpdateRequested();
    void PersistentHiddenToggled(bool hidden);

private:
    // === Initialization ===
    void CreateTrayIcon();
    void OnTrayActivated(QSystemTrayIcon::ActivationReason Reason);

    // === Data ===
    QSystemTrayIcon* TrayIcon          = nullptr;
    QMenu*           TrayMenu          = nullptr;
    QAction*         PauseAction       = nullptr;
    QAction*         UpdateAction      = nullptr;
    QAction*         PersistentAction  = nullptr;
    
    // === State ===
    bool             bIsPaused         = false;
};

} // namespace gpm
