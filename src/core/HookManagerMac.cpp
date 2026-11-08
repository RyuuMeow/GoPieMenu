#include "HookManager.h"

#ifdef Q_OS_MACOS

#include "platform/mac/MacApplications.h"
#include "platform/mac/MacKeyMap.h"

// Accessibility and Input Monitoring permission APIs live in HIServices, which
// this framework umbrella exposes; the CoreGraphics event APIs come with
// HookManager.h.
#include <ApplicationServices/ApplicationServices.h>

#include <QCursor>
#include <QDebug>

#include <array>
#include <cstdint>
#include <vector>

namespace gpm
{
namespace
{
// Events the runtime consumes. CoreGraphics turns this into the bit mask it
// expects; kCGEventTapDisabled* notifications arrive without being requested.
constexpr CGEventMask TapMask =
    CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp) |
    CGEventMaskBit(kCGEventFlagsChanged) |
    CGEventMaskBit(kCGEventLeftMouseDown) | CGEventMaskBit(kCGEventLeftMouseUp) |
    CGEventMaskBit(kCGEventRightMouseDown) | CGEventMaskBit(kCGEventRightMouseUp) |
    CGEventMaskBit(kCGEventOtherMouseDown) | CGEventMaskBit(kCGEventOtherMouseUp) |
    CGEventMaskBit(kCGEventMouseMoved) |
    CGEventMaskBit(kCGEventLeftMouseDragged) | CGEventMaskBit(kCGEventRightMouseDragged) |
    CGEventMaskBit(kCGEventOtherMouseDragged) |
    CGEventMaskBit(kCGEventScrollWheel);

// Windows delivers one 120-unit wheel delta per notch. Continuous devices
// (trackpads, Magic Mouse) deliver many small pixel deltas, so they are
// accumulated until they add up to one readable row step.
constexpr int WheelDeltaPerNotch = 120;
constexpr int PointDeltaPerNotch = 32;

// Device-dependent modifier bits reported in CGEventFlags. These are the
// NX_DEVICEL*MASK values from <IOKit/hidsystem/IOLLEvent.h>; they are repeated
// here so this file does not depend on an IOKit include.
constexpr CGEventFlags DeviceLeftControl  = 0x00000001;
constexpr CGEventFlags DeviceLeftShift    = 0x00000002;
constexpr CGEventFlags DeviceRightShift   = 0x00000004;
constexpr CGEventFlags DeviceLeftCommand  = 0x00000008;
constexpr CGEventFlags DeviceRightCommand = 0x00000010;
constexpr CGEventFlags DeviceLeftOption   = 0x00000020;
constexpr CGEventFlags DeviceRightOption  = 0x00000040;
constexpr CGEventFlags DeviceRightControl = 0x00002000;

constexpr CGEventFlags DeviceModifierMasks =
    DeviceLeftControl | DeviceLeftShift | DeviceRightShift | DeviceLeftCommand |
    DeviceRightCommand | DeviceLeftOption | DeviceRightOption | DeviceRightControl;

struct ModifierKey
{
    int MacKeyCode;
    int WindowsVk;
    CGEventFlags Generic;
    CGEventFlags Device;
};

// Sided modifiers stay independent in InputState, so the physical key code and
// its device-dependent flag are both needed. Generic flags alone cannot tell
// which side is released while the other side is still held.
constexpr ModifierKey ModifierKeys[] = {
    {0x37, 0x5b, kCGEventFlagMaskCommand,   DeviceLeftCommand},   // kVK_Command
    {0x36, 0x5c, kCGEventFlagMaskCommand,   DeviceRightCommand},  // kVK_RightCommand
    {0x38, 0xa0, kCGEventFlagMaskShift,     DeviceLeftShift},     // kVK_Shift
    {0x3c, 0xa1, kCGEventFlagMaskShift,     DeviceRightShift},    // kVK_RightShift
    {0x3b, 0xa2, kCGEventFlagMaskControl,   DeviceLeftControl},   // kVK_Control
    {0x3e, 0xa3, kCGEventFlagMaskControl,   DeviceRightControl},  // kVK_RightControl
    {0x3a, 0xa4, kCGEventFlagMaskAlternate, DeviceLeftOption},    // kVK_Option
    {0x3d, 0xa5, kCGEventFlagMaskAlternate, DeviceRightOption},   // kVK_RightOption
};

// Caps Lock also arrives as a flags-changed event, but it is a locking key: the
// alpha-shift flag is its toggled state rather than a held key.
constexpr int MacCapsLockKeyCode = 0x39;
constexpr int WindowsVkCapsLock = 0x14;

// The shared virtual-key codes for the modifiers used by the input state.
constexpr int VkShift = 0x10;
constexpr int VkControl = 0x11;
constexpr int VkAlt = 0x12;

bool IsOwnEvent(CGEventRef event)
{
    return CGEventGetIntegerValueField(event, kCGEventSourceUserData) ==
        static_cast<int64_t>(OwnInputTag);
}
} // namespace

HookManager* HookManager::Instance = nullptr;

HookManager::HookManager(ConfigManager* manager, QObject* parent) : IHookProvider(parent), Manager(manager)
{
    Instance = this;
    State.setProfiles(Manager->GetConfig().Profiles);
    connect(Manager, &ConfigManager::ConfigChanged, this, &HookManager::Reset);
}

HookManager::~HookManager()
{
    Uninstall();
    if (Instance == this) Instance = nullptr;
}

void HookManager::synchronizeKeys()
{
    // A release of a key that was already down when the tap was installed must
    // reach the application, so mirror the Windows GetAsyncKeyState sweep. There
    // is no per-virtual-key query on macOS, so every physical key code is
    // queried and ORed into the shared virtual key it maps to.
    std::array<bool, 256> down{};
    for (const auto& mapping : MacKeyMappings())
    {
        if (mapping.WindowsVk == VkShift || mapping.WindowsVk == VkControl || mapping.WindowsVk == VkAlt)
            continue;
        if (CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState,
                                  static_cast<CGKeyCode>(mapping.MacKeyCode)))
            down[static_cast<size_t>(mapping.WindowsVk)] = true;
    }
    for (int vk = 0; vk < 256; ++vk)
        State.synchronizeKey(vk, down[static_cast<size_t>(vk)]);
}

bool HookManager::Install()
{
    if (IsInstalled()) return true;
    State.reset();
    State.setProfiles(Manager->GetConfig().Profiles);

    // An active event tap and synthetic event posting both require the
    // Accessibility permission. Prompt once; granting it needs a relaunch before
    // TCC applies to this process, so report the failure to the UI instead of
    // pretending the hook is live.
    const void* keys[] = {kAXTrustedCheckOptionPrompt};
    const void* values[] = {kCFBooleanTrue};
    CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    const Boolean trusted = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    if (!trusted) return false;

    synchronizeKeys();

    Tap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
                           TapMask, &HookManager::EventTapCallback, this);
    if (!Tap)
    {
        // kCGHIDEventTap is documented as root-only but works for trusted
        // accessibility clients. Some macOS versions/compat environments only
        // allow the session-level tap, which filters the same runtime events.
        Tap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
                               TapMask, &HookManager::EventTapCallback, this);
    }
    if (!Tap)
    {
        // On macOS 10.15+ a tap that observes keyboard events also needs the
        // Input Monitoring permission. Ask for it while reporting the failure;
        // the user must restart the application after granting it.
        if (!CGPreflightListenEventAccess()) CGRequestListenEventAccess();
        return false;
    }

    TapSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, Tap, 0);
    if (!TapSource)
    {
        CFRelease(Tap);
        Tap = nullptr;
        return false;
    }
    // The tap callback runs on the run loop it is added to. The manager lives on
    // the UI thread, so the main run loop keeps the signals and InputState
    // single-threaded, exactly like the Windows low-level hooks.
    CFRunLoopAddSource(CFRunLoopGetMain(), TapSource, kCFRunLoopCommonModes);
    CGEventTapEnable(Tap, true);

    MacSetForegroundChangeHandler([this] {
        Cancel();
        emit ForegroundChanged(foregroundProcess());
    });
    emit ForegroundChanged(foregroundProcess());
    return true;
}

void HookManager::Uninstall()
{
    Cancel();
    MacClearForegroundChangeHandler();
    if (TapSource)
    {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), TapSource, kCFRunLoopCommonModes);
        CFRelease(TapSource);
        TapSource = nullptr;
    }
    if (Tap)
    {
        CGEventTapEnable(Tap, false);
        CFMachPortInvalidate(Tap);
        CFRelease(Tap);
        Tap = nullptr;
    }
    ScrollAccumulator = 0;
    State.reset();
}

bool HookManager::IsInstalled() const
{
    return Tap != nullptr;
}

void HookManager::Cancel()
{
    dispatch(State.cancel());
}

void HookManager::Reset()
{
    Cancel();
    State.setProfiles(Manager->GetConfig().Profiles);
    if (IsInstalled()) synchronizeKeys();
}

void HookManager::SetSuspended(bool value)
{
    if (Suspended == value) return;
    Suspended = value;
    Reset();
}

QString HookManager::foregroundProcess() const
{
    return MacForegroundProcessName();
}

void HookManager::dispatch(const InputDecision& d)
{
    switch (d.Effect)
    {
        case InputEffect::Show:     emit Triggered(d.ProfileId, QCursor::pos()); break;
        case InputEffect::Confirm:  emit TriggerReleased(QCursor::pos()); break;
        case InputEffect::Cancel:   emit CancelRequested(); break;
        case InputEffect::Move:     emit MouseMoved(QCursor::pos()); break;
        case InputEffect::Scroll:   emit WheelScrolled(d.WheelDelta); break;
        default: break;
    }
}

CGEventRef HookManager::EventTapCallback(CGEventTapProxy, CGEventType type, CGEventRef event, void* userInfo)
{
    auto* manager = static_cast<HookManager*>(userInfo);
    if (!manager) return event;
    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput)
    {
        // The window server disables a tap whose callback was too slow. Windows
        // low-level hooks have no such watchdog, so recover transparently.
        if (manager->Tap) CGEventTapEnable(manager->Tap, true);
        return event;
    }
    // Returning null removes the event from the stream; returning it unchanged
    // lets the system deliver it. Active filters discard matched trigger input.
    return manager->handleEvent(type, event) ? nullptr : event;
}

bool HookManager::handleEvent(CGEventType type, CGEventRef event)
{
    if (!event) return false;
    if (Suspended) return false;
    // SendHotkeyAction tags the events it posts through the event source; those
    // events must reach the target application without driving the state machine.
    if (IsOwnEvent(event)) return false;

    switch (type)
    {
        case kCGEventKeyDown:
        case kCGEventKeyUp:
        {
            const int macKeyCode = static_cast<int>(
                CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
            const int vk = WindowsVkFromMacKeyCode(macKeyCode);
            if (vk < 0) return false;
            const bool down = type == kCGEventKeyDown;
            const auto process = down && !State.active() ? foregroundProcess() : QString();
            const auto result = State.key(vk, down, process, false);
            dispatch(result);
            return result.Swallow;
        }
        case kCGEventFlagsChanged:
            return handleFlagsChanged(event);
        case kCGEventLeftMouseDown:
        case kCGEventLeftMouseUp:
        case kCGEventRightMouseDown:
        case kCGEventRightMouseUp:
        case kCGEventOtherMouseDown:
        case kCGEventOtherMouseUp:
        {
            bool down = false;
            MouseButton button = MouseButton::None;
            switch (type)
            {
                case kCGEventLeftMouseDown:  button = MouseButton::Left; down = true; break;
                case kCGEventLeftMouseUp:    button = MouseButton::Left; break;
                case kCGEventRightMouseDown: button = MouseButton::Right; down = true; break;
                case kCGEventRightMouseUp:   button = MouseButton::Right; break;
                case kCGEventOtherMouseDown: down = true; [[fallthrough]];
                case kCGEventOtherMouseUp:
                {
                    // Button numbers follow the CGEvent convention: 0 left,
                    // 1 right, 2 middle, then USB order for the side buttons.
                    const auto number = CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);
                    if (number == 2) button = MouseButton::Middle;
                    else if (number == 3) button = MouseButton::X1;
                    else if (number == 4) button = MouseButton::X2;
                    break;
                }
                default: break;
            }
            if (button == MouseButton::None) return false;
            // Runtime window geometry uses Qt logical coordinates, not native pixels.
            if (down) emit PointerPressed(QCursor::pos());
            const auto result = State.mouse(button, down,
                down && !State.active() ? foregroundProcess() : QString());
            dispatch(result);
            return result.Swallow;
        }
        case kCGEventMouseMoved:
        case kCGEventLeftMouseDragged:
        case kCGEventRightMouseDragged:
        case kCGEventOtherMouseDragged:
        {
            const auto result = State.move();
            dispatch(result);
            return result.Swallow;
        }
        case kCGEventScrollWheel:
        {
            int delta = 0;
            if (CGEventGetIntegerValueField(event, kCGScrollWheelEventIsContinuous) != 0)
            {
                const auto points = static_cast<int>(
                    CGEventGetIntegerValueField(event, kCGScrollWheelEventPointDeltaAxis1));
                if (points != 0)
                {
                    // Reverse direction starts a new gesture instead of paying
                    // off the previous accumulation.
                    if ((ScrollAccumulator > 0 && points < 0) || (ScrollAccumulator < 0 && points > 0))
                        ScrollAccumulator = 0;
                    ScrollAccumulator += points;
                    while (ScrollAccumulator >= PointDeltaPerNotch)
                    {
                        ScrollAccumulator -= PointDeltaPerNotch;
                        delta += WheelDeltaPerNotch;
                    }
                    while (ScrollAccumulator <= -PointDeltaPerNotch)
                    {
                        ScrollAccumulator += PointDeltaPerNotch;
                        delta -= WheelDeltaPerNotch;
                    }
                }
            }
            else
            {
                delta = static_cast<int>(
                    CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1)) * WheelDeltaPerNotch;
            }
            const auto result = State.wheel(delta);
            if (delta != 0) dispatch(result);
            return result.Swallow;
        }
        default:
            return false;
    }
}

bool HookManager::handleFlagsChanged(CGEventRef event)
{
    const int macKeyCode = static_cast<int>(
        CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    const CGEventFlags flags = CGEventGetFlags(event);
    int vk = -1;
    bool down = false;
    if (macKeyCode == MacCapsLockKeyCode)
    {
        vk = WindowsVkCapsLock;
        down = (flags & kCGEventFlagMaskAlphaShift) != 0;
    }
    else
    {
        for (const auto& key : ModifierKeys)
        {
            if (key.MacKeyCode != macKeyCode) continue;
            vk = key.WindowsVk;
            // The device flag identifies this side even while the other side
            // keeps the generic flag set. Fall back to the generic flag if a
            // platform build does not report device bits.
            down = (flags & DeviceModifierMasks) != 0
                ? (flags & key.Device) != 0
                : (flags & key.Generic) != 0;
            break;
        }
    }
    if (vk < 0) return false; // Fn and other flag changes have no shared VK.
    const auto process = down && !State.active() ? foregroundProcess() : QString();
    const auto result = State.key(vk, down, process, false);
    dispatch(result);
    return result.Swallow;
}

} // namespace gpm

#endif // Q_OS_MACOS
