#pragma once

// =============================================================================
// GoPieMenu - HookManager (Win32 Low-Level Hooks)
// =============================================================================

#include "IHookProvider.h"
#include "ConfigManager.h"

#include <windows.h>
#include <QPoint>
#include <QTimer>

#include <atomic>

namespace gpm 
{

class HookManager : public IHookProvider 
{
    Q_OBJECT
public:
    explicit HookManager(ConfigManager* InConfigMgr, QObject* Parent = nullptr);
    virtual ~HookManager() override;

    virtual bool Install() override;
    virtual void Uninstall() override;
    [[nodiscard]] virtual bool IsInstalled() const override;

private:
    // === Win32 Callbacks ===
    static LRESULT CALLBACK KeyboardProc(int NCode, WPARAM WParam, LPARAM LParam);
    static LRESULT CALLBACK MouseProc(int NCode, WPARAM WParam, LPARAM LParam);

    // === Event Handling ===
    bool OnKeyEvent(WPARAM WParam, const KBDLLHOOKSTRUCT& KB);
    bool OnMouseEvent(WPARAM WParam, const MSLLHOOKSTRUCT& MS);

    // === State Management ===
    void UpdateModifierState(DWORD VKCode, bool bPressed);
    [[nodiscard]] ModifierKey GetCurrentModifiers() const;

    // === Helpers ===
    [[nodiscard]] QString GetForegroundProcessName() const;
    [[nodiscard]] bool MatchesAppFilter(const PieMenuConfig& Profile) const;
    [[nodiscard]] const PieMenuConfig* FindMatchingProfile(MouseButton Btn, uint32_t VKCode = 0) const;

    // === Members ===
    ConfigManager* ConfigMgr = nullptr;
    HHOOK          KBHook    = nullptr;
    HHOOK          MSHook    = nullptr;

    // === Modifier State ===
    std::atomic<bool> bCtrlDown  = false;
    std::atomic<bool> bShiftDown = false;
    std::atomic<bool> bAltDown   = false;
    std::atomic<bool> bWinDown   = false;

    // === Trigger State ===
    std::atomic<bool> bTriggerActive = false;
    QString           ActiveProfileId;

    // === Singleton ===
    static HookManager* Instance;
};

} // namespace gpm
