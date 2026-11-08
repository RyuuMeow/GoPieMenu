#pragma once
#include "IHookProvider.h"
#include "ConfigManager.h"
#include "InputState.h"
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MACOS)
// Narrow CoreGraphics/CoreFoundation headers keep this header usable from plain
// C++ translation units; the implementation adds ApplicationServices for the
// Accessibility checks it alone needs.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGEvent.h>
#endif

namespace gpm {
// Global input adapter used by the runtime. Windows uses low-level hooks; macOS
// uses a CoreGraphics event tap. Both feed the same platform-independent
// InputState machine and emit the same signals.
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
    void dispatch(const InputDecision& decision);
    void synchronizeKeys();
    ConfigManager* Manager;
    InputState State;
    bool Suspended = false;
    static HookManager* Instance;
#ifdef Q_OS_WIN
    static void CALLBACK ForegroundProc(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
    static LRESULT CALLBACK KeyboardProc(int code, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK MouseProc(int code, WPARAM wparam, LPARAM lparam);
    HHOOK Keyboard = nullptr, Mouse = nullptr;
    HWINEVENTHOOK Foreground = nullptr;
    HWND PendingForeground = nullptr;
    QTimer ForegroundUpdate;
#elif defined(Q_OS_MACOS)
    static CGEventRef EventTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* userInfo);
    bool handleEvent(CGEventType type, CGEventRef event);
    bool handleFlagsChanged(CGEventRef event);
    CFMachPortRef Tap = nullptr;
    CFRunLoopSourceRef TapSource = nullptr;
    // Trackpads deliver continuous pixel scrolling; accumulate it so the list
    // menu advances by whole wheel steps instead of one row per pixel event.
    int ScrollAccumulator = 0;
#endif
};
}
