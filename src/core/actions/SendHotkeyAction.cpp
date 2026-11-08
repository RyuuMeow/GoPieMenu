// =============================================================================
// GoPieMenu - SendHotkeyAction Implementation
// =============================================================================

#include "SendHotkeyAction.h"
#include "core/InputState.h"
#include "core/Shortcut.h"

#include <QDebug>
#include <QStringList>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <ApplicationServices/ApplicationServices.h>
#include "platform/mac/MacKeyMap.h"
#endif

namespace gpm 
{

#ifdef Q_OS_WIN

bool SendHotkeyAction::Execute(const PieItem& Item) 
{
    qDebug() << "[SendHotkey] Sending:" << Item.ActionData;

    auto Keys = ParseShortcut(Item.ActionData);
    if (Keys.empty()) 
    {
        qWarning() << "[SendHotkey] Failed to parse hotkey:" << Item.ActionData;
        return false;
    }

    auto HeldMods = GetHeldModifiers();
    
    // Build INPUT array
    std::vector<INPUT> Inputs;
    Inputs.reserve(HeldMods.size() * 2 + Keys.size() * 2);

    // 1. Release currently held physical modifiers
    for (auto VK : HeldMods) 
    {
        INPUT Inp{};
        Inp.type       = INPUT_KEYBOARD;
        Inp.ki.wVk     = static_cast<WORD>(VK);
        Inp.ki.wScan   = static_cast<WORD>(MapVirtualKeyW(VK, MAPVK_VK_TO_VSC));
        Inp.ki.dwFlags = KEYEVENTF_KEYUP | (bIsExtendedKey(VK) ? KEYEVENTF_EXTENDEDKEY : 0);
        Inputs.push_back(Inp);
    }

    // 2. Press our hotkey keys
    for (auto VK : Keys) 
    {
        INPUT Inp{};
        Inp.type       = INPUT_KEYBOARD;
        Inp.ki.wVk     = static_cast<WORD>(VK);
        Inp.ki.wScan   = static_cast<WORD>(MapVirtualKeyW(VK, MAPVK_VK_TO_VSC));
        Inp.ki.dwFlags = bIsExtendedKey(VK) ? KEYEVENTF_EXTENDEDKEY : 0;
        Inputs.push_back(Inp);
    }

    // 3. Release our hotkey keys
    for (auto It = Keys.rbegin(); It != Keys.rend(); ++It) 
    {
        INPUT Inp{};
        Inp.type       = INPUT_KEYBOARD;
        Inp.ki.wVk     = static_cast<WORD>(*It);
        Inp.ki.wScan   = static_cast<WORD>(MapVirtualKeyW(*It, MAPVK_VK_TO_VSC));
        Inp.ki.dwFlags = KEYEVENTF_KEYUP | (bIsExtendedKey(*It) ? KEYEVENTF_EXTENDEDKEY : 0);
        Inputs.push_back(Inp);
    }

    // 4. Re-press the physical modifiers so the OS state matches reality
    for (auto It = HeldMods.rbegin(); It != HeldMods.rend(); ++It) 
    {
        INPUT Inp{};
        Inp.type       = INPUT_KEYBOARD;
        Inp.ki.wVk     = static_cast<WORD>(*It);
        Inp.ki.wScan   = static_cast<WORD>(MapVirtualKeyW(*It, MAPVK_VK_TO_VSC));
        Inp.ki.dwFlags = bIsExtendedKey(*It) ? KEYEVENTF_EXTENDEDKEY : 0;
        Inputs.push_back(Inp);
    }

    for (auto& input : Inputs) input.ki.dwExtraInfo = OwnInputTag;
    UINT Sent = SendInput(static_cast<UINT>(Inputs.size()), Inputs.data(), sizeof(INPUT));
    return Sent == Inputs.size();
}

std::vector<int> SendHotkeyAction::GetHeldModifiers() 
{
    std::vector<int> Held;
    for (int VK : {VK_LCONTROL, VK_RCONTROL, VK_LSHIFT, VK_RSHIFT, VK_LMENU, VK_RMENU, VK_LWIN, VK_RWIN}) 
    {
        if (GetAsyncKeyState(VK) & 0x8000) 
        {
            Held.push_back(VK);
        }
    }
    return Held;
}

bool SendHotkeyAction::bIsExtendedKey(int VK) 
{
    return VK == VK_RCONTROL || VK == VK_RMENU  ||
           VK == VK_INSERT   || VK == VK_DELETE ||
           VK == VK_HOME     || VK == VK_END    ||
           VK == VK_PRIOR    || VK == VK_NEXT   ||
           VK == VK_UP       || VK == VK_DOWN   ||
           VK == VK_LEFT     || VK == VK_RIGHT  ||
           VK == VK_LWIN     || VK == VK_RWIN   ||
           VK == VK_SNAPSHOT;
}

#elif defined(Q_OS_MACOS)

namespace 
{
// Generic Quartz modifier mask for the shared sided virtual-key codes.
CGEventFlags ModifierMaskForVk(int Vk) 
{
    switch (Vk) 
    {
        case 0xa0: case 0xa1: return kCGEventFlagMaskShift;
        case 0xa2: case 0xa3: return kCGEventFlagMaskControl;
        case 0xa4: case 0xa5: return kCGEventFlagMaskAlternate;
        case 0x5b: case 0x5c: return kCGEventFlagMaskCommand;
        default:              return CGEventFlags(0);
    }
}

void PostKeyboardEvent(CGEventSourceRef Source, int MacKeyCode, bool Down, CGEventFlags Flags) 
{
    CGEventRef Event = CGEventCreateKeyboardEvent(Source, CGKeyCode(MacKeyCode), Down);
    if (!Event) return;
    CGEventSetFlags(Event, Flags);
    CGEventPost(kCGHIDEventTap, Event);
    CFRelease(Event);
}
} // namespace

bool SendHotkeyAction::Execute(const PieItem& Item) 
{
    qDebug() << "[SendHotkey] Sending:" << Item.ActionData;

    auto Keys = ParseShortcut(Item.ActionData);
    if (Keys.empty()) 
    {
        qWarning() << "[SendHotkey] Failed to parse hotkey:" << Item.ActionData;
        return false;
    }

    // Posting events that other applications receive requires Accessibility
    // trust. Do not prompt here: the action simply fails like a Windows
    // SendInput call that the desktop rejects.
    if (!AXIsProcessTrusted()) 
    {
        qWarning() << "[SendHotkey] Accessibility permission is required to send keys";
        return false;
    }

    // Resolve every key before posting anything so a key that has no physical
    // macOS counterpart cannot leave a half-sent chord behind.
    std::vector<int> MacKeys;
    MacKeys.reserve(Keys.size());
    for (int Vk : Keys) 
    {
        const int Code = MacKeyCodeFromWindowsVk(Vk);
        if (Code < 0) 
        {
            qWarning() << "[SendHotkey] No macOS key code for virtual key" << Vk;
            return false;
        }
        MacKeys.push_back(Code);
    }

    CGEventSourceRef Source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    if (!Source) return false;
    // Events created from this source inherit the tag, so our own event tap
    // recognizes them as self-generated input and never re-triggers a menu.
    CGEventSourceSetUserData(Source, static_cast<int64_t>(OwnInputTag));
    CGEventFlags Flags = CGEventSourceFlagsState(kCGEventSourceStateCombinedSessionState);

    // 1. Release currently held physical modifiers. Their sided state cannot be
    //    read from the combined flags, so query each physical modifier key.
    const auto HeldMods = GetHeldModifiers();
    for (int Vk : HeldMods) 
    {
        const int Code = MacKeyCodeFromWindowsVk(Vk);
        if (Code < 0) continue;
        Flags &= ~ModifierMaskForVk(Vk);
        PostKeyboardEvent(Source, Code, false, Flags);
    }

    // 2. Press our hotkey keys. ParseShortcut returns the modifiers first, so
    //    the main key event carries the intended modifier flags.
    for (size_t Index = 0; Index < Keys.size(); ++Index) 
    {
        Flags |= ModifierMaskForVk(Keys[Index]);
        PostKeyboardEvent(Source, MacKeys[Index], true, Flags);
    }

    // 3. Release our hotkey keys in reverse order.
    for (size_t Index = Keys.size(); Index-- > 0;) 
    {
        const auto Mask = ModifierMaskForVk(Keys[Index]);
        if (Mask != 0) Flags &= ~Mask;
        PostKeyboardEvent(Source, MacKeys[Index], false, Flags);
    }

    // 4. Re-press the physical modifiers so the OS state matches reality.
    for (auto It = HeldMods.rbegin(); It != HeldMods.rend(); ++It) 
    {
        const int Code = MacKeyCodeFromWindowsVk(*It);
        if (Code < 0) continue;
        Flags |= ModifierMaskForVk(*It);
        PostKeyboardEvent(Source, Code, true, Flags);
    }

    CFRelease(Source);
    return true;
}

std::vector<int> SendHotkeyAction::GetHeldModifiers() 
{
    // CGEventSourceFlagsState is not sided, so read every physical modifier key.
    // The virtual-key order matches the Windows implementation.
    static const int MacKeyCodes[] = {0x3b, 0x3e, 0x38, 0x3c, 0x3a, 0x3d, 0x37, 0x36};
    static const int WindowsVks[]  = {0xa2, 0xa3, 0xa0, 0xa1, 0xa4, 0xa5, 0x5b, 0x5c};
    std::vector<int> Held;
    for (size_t Index = 0; Index < sizeof(MacKeyCodes) / sizeof(MacKeyCodes[0]); ++Index) 
    {
        if (CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, CGKeyCode(MacKeyCodes[Index]))) 
        {
            Held.push_back(WindowsVks[Index]);
        }
    }
    return Held;
}

#else

// No input injection backend on other platforms; keep the action registered but
// unable to send anything.
bool SendHotkeyAction::Execute(const PieItem&) 
{
    return false;
}

std::vector<int> SendHotkeyAction::GetHeldModifiers() 
{
    return {};
}

#endif

} // namespace gpm
