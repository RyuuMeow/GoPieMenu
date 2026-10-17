#pragma once
#include "IHookProvider.h"
#include "ConfigManager.h"
#include "InputState.h"
#include <windows.h>

namespace gpm {
class HookManager : public IHookProvider {
    Q_OBJECT
public:
    explicit HookManager(ConfigManager* manager, QObject* parent = nullptr);
    ~HookManager() override;
    bool Install() override;
    void Uninstall() override;
    bool IsInstalled() const override;
    void SetSuspended(bool suspended);
    void Reset();
    void Cancel();
    bool IsSuspended() const { return Suspended; }
    QString foregroundProcess() const;
signals:
    void WheelScrolled(int delta);
    void ForegroundChanged(const QString& process);
    void PointerPressed(const QPoint& point);
private:
    static void CALLBACK ForegroundProc(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
    static LRESULT CALLBACK KeyboardProc(int code, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK MouseProc(int code, WPARAM wparam, LPARAM lparam);
    void dispatch(const InputDecision& decision);
    void synchronizeKeys();
    ConfigManager* Manager;
    InputState State;
    HHOOK Keyboard = nullptr, Mouse = nullptr;
    HWINEVENTHOOK Foreground = nullptr;
    bool Suspended = false;
    static HookManager* Instance;
};
}
