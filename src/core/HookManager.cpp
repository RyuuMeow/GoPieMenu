#include "HookManager.h"
#include <QCursor>
#include <QFileInfo>
#include <QDebug>

namespace gpm {
HookManager* HookManager::Instance = nullptr;
HookManager::HookManager(ConfigManager* manager, QObject* parent) : IHookProvider(parent), Manager(manager) {
    Instance = this; State.setProfiles(Manager->GetConfig().Profiles);
    connect(Manager, &ConfigManager::ConfigChanged, this, &HookManager::Reset);
}
HookManager::~HookManager() { Uninstall(); if (Instance == this) Instance = nullptr; }
void HookManager::synchronizeKeys() {
    // Only synchronize sided modifiers, avoiding a stale generic Ctrl flag.
    for (int key = 0; key < 256; ++key) {
        if (key != VK_SHIFT && key != VK_CONTROL && key != VK_MENU)
            State.synchronizeKey(key, (GetAsyncKeyState(key) & 0x8000) != 0);
    }
}
bool HookManager::Install() {
    if (IsInstalled()) return true;
    State.reset(); State.setProfiles(Manager->GetConfig().Profiles); synchronizeKeys();
    Keyboard = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandleW(nullptr), 0);
    if (!Keyboard) return false;
    Mouse = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, GetModuleHandleW(nullptr), 0);
    if (!Mouse) { UnhookWindowsHookEx(Keyboard); Keyboard = nullptr; return false; }
    return true;
}
void HookManager::Uninstall() {
    Cancel();
    if (Keyboard) UnhookWindowsHookEx(Keyboard);
    if (Mouse) UnhookWindowsHookEx(Mouse);
    Keyboard = nullptr; Mouse = nullptr; State.reset();
}
bool HookManager::IsInstalled() const { return Keyboard && Mouse; }
void HookManager::Cancel() { dispatch(State.cancel()); }
void HookManager::Reset() {
    Cancel(); State.setProfiles(Manager->GetConfig().Profiles);
    if (IsInstalled()) synchronizeKeys();
}
void HookManager::SetSuspended(bool value) {
    if (Suspended == value) return;
    Suspended = value; Reset();
}
QString HookManager::foregroundProcess() const {
    const auto window = GetForegroundWindow();
    if (!window) return {};
    DWORD pid = 0; GetWindowThreadProcessId(window, &pid);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return {};
    wchar_t path[32768]{}; DWORD length = 32768;
    const bool ok = QueryFullProcessImageNameW(process, 0, path, &length);
    CloseHandle(process);
    return ok ? QFileInfo(QString::fromWCharArray(path, int(length))).fileName() : QString();
}
void HookManager::dispatch(const InputDecision& d) {
    switch (d.Effect) {
    case InputEffect::Show: emit Triggered(d.ProfileId, QCursor::pos()); break;
    case InputEffect::Confirm: emit TriggerReleased(QCursor::pos()); break;
    case InputEffect::Cancel: emit CancelRequested(); break;
    case InputEffect::Move: emit MouseMoved(QCursor::pos()); break;
    case InputEffect::Scroll: emit WheelScrolled(d.WheelDelta); break;
    default: break;
    }
}
LRESULT CALLBACK HookManager::KeyboardProc(int code, WPARAM wparam, LPARAM lparam) {
    if (code >= 0 && Instance && !Instance->Suspended) {
        const auto& key = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(lparam);
        const bool down = wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN;
        const bool own = key.dwExtraInfo == OwnInputTag;
        const auto process = down && !Instance->State.active() && !own ? Instance->foregroundProcess() : QString();
        const auto result = Instance->State.key(int(key.vkCode), down, process, own);
        Instance->dispatch(result);
        if (result.Swallow) return 1;
    }
    return CallNextHookEx(nullptr, code, wparam, lparam);
}
LRESULT CALLBACK HookManager::MouseProc(int code, WPARAM wparam, LPARAM lparam) {
    if (code >= 0 && Instance && !Instance->Suspended) {
        const auto& mouse = *reinterpret_cast<const MSLLHOOKSTRUCT*>(lparam);
        if (mouse.dwExtraInfo == OwnInputTag) return CallNextHookEx(nullptr, code, wparam, lparam);
        InputDecision result;
        if (wparam == WM_MOUSEMOVE) result = Instance->State.move();
        else if (wparam == WM_MOUSEWHEEL) result = Instance->State.wheel(short(HIWORD(mouse.mouseData)));
        else {
            MouseButton button = MouseButton::None; bool down = false;
            switch (wparam) {
            case WM_LBUTTONDOWN: button = MouseButton::Left; down = true; break;
            case WM_LBUTTONUP: button = MouseButton::Left; break;
            case WM_RBUTTONDOWN: button = MouseButton::Right; down = true; break;
            case WM_RBUTTONUP: button = MouseButton::Right; break;
            case WM_MBUTTONDOWN: button = MouseButton::Middle; down = true; break;
            case WM_MBUTTONUP: button = MouseButton::Middle; break;
            case WM_XBUTTONDOWN: down = true; [[fallthrough]];
            case WM_XBUTTONUP: button = HIWORD(mouse.mouseData) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2; break;
            }
            result = Instance->State.mouse(button, down, down && !Instance->State.active() ? Instance->foregroundProcess() : QString());
        }
        Instance->dispatch(result);
        if (result.Swallow) return 1;
    }
    return CallNextHookEx(nullptr, code, wparam, lparam);
}
}
