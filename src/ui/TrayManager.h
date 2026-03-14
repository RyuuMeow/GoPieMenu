#pragma once

// =============================================================================
// GoPieMenu - System Tray Manager
// =============================================================================

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

namespace gpm 
{

class SettingsWindow;

class TrayManager : public QObject 
{
    Q_OBJECT

public:
    explicit TrayManager(QObject* Parent = nullptr);
    virtual ~TrayManager() override;

    /** Makes the tray icon visible */
    void Show();
    
    /** Link to the settings window for single-instance control */
    void SetSettingsWindow(SettingsWindow* InWindow) { SettingsWindowPtr = InWindow; }

signals:
    void SettingsRequested();
    void PauseToggled(bool bPaused);
    void QuitRequested();

private:
    // === Initialization ===
    void CreateTrayIcon();
    void OnTrayActivated(QSystemTrayIcon::ActivationReason Reason);

    // === Data ===
    QSystemTrayIcon* TrayIcon          = nullptr;
    QMenu*           TrayMenu          = nullptr;
    QAction*         PauseAction       = nullptr;
    SettingsWindow*  SettingsWindowPtr = nullptr;
    
    // === State ===
    bool             bIsPaused         = false;
};

} // namespace gpm
