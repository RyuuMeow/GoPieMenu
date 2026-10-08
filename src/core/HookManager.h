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
signals:
    void WheelScrolled(int delta);
private:
    static LRESULT CALLBACK KeyboardProc(int code, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK MouseProc(int code, WPARAM wparam, LPARAM lparam);
    void dispatch(const InputDecision& decision);
    QString foregroundProcess() const;
    void synchronizeKeys();
    ConfigManager* Manager;
    InputState State;
    HHOOK Keyboard = nullptr, Mouse = nullptr;
    bool Suspended = false;
    static HookManager* Instance;
};
}
