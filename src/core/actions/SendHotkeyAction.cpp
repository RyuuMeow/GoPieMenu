// =============================================================================
// GoPieMenu - SendHotkeyAction Implementation
// =============================================================================

#include "SendHotkeyAction.h"
#include "core/InputState.h"
#include "core/Shortcut.h"

#include <QDebug>
#include <QStringList>
#include <windows.h>

namespace gpm 
{

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

} // namespace gpm
