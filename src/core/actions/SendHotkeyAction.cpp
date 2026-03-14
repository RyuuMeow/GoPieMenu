// =============================================================================
// GoPieMenu - SendHotkeyAction Implementation
// =============================================================================

#include "SendHotkeyAction.h"

#include <QDebug>
#include <QStringList>
#include <windows.h>

namespace gpm 
{

bool SendHotkeyAction::Execute(const PieItem& Item) 
{
    qDebug() << "[SendHotkey] Sending:" << Item.ActionData;

    auto Keys = ParseHotkey(Item.ActionData);
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

std::vector<int> SendHotkeyAction::ParseHotkey(const QString& InString) 
{
    static const std::unordered_map<QString, int> KeyMap = 
    {
        {"ctrl",      VK_LCONTROL},
        {"control",   VK_LCONTROL},
        {"shift",     VK_LSHIFT},
        {"alt",       VK_LMENU},
        {"win",       VK_LWIN},
        {"tab",       VK_TAB},
        {"enter",     VK_RETURN},
        {"return",    VK_RETURN},
        {"escape",    VK_ESCAPE},
        {"esc",       VK_ESCAPE},
        {"space",     VK_SPACE},
        {"backspace", VK_BACK},
        {"delete",    VK_DELETE},
        {"del",       VK_DELETE},
        {"insert",    VK_INSERT},
        {"ins",       VK_INSERT},
        {"home",      VK_HOME},
        {"end",       VK_END},
        {"pageup",    VK_PRIOR},
        {"pgup",      VK_PRIOR},
        {"pagedown",  VK_NEXT},
        {"pgdn",      VK_NEXT},
        {"up",        VK_UP},
        {"down",      VK_DOWN},
        {"left",      VK_LEFT},
        {"right",     VK_RIGHT},
        {"f1",  VK_F1},  {"f2",  VK_F2},  {"f3",  VK_F3},  {"f4",  VK_F4},
        {"f5",  VK_F5},  {"f6",  VK_F6},  {"f7",  VK_F7},  {"f8",  VK_F8},
        {"f9",  VK_F9},  {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12},
        {"printscreen", VK_SNAPSHOT},
        {"prtsc",       VK_SNAPSHOT},
        {"scrolllock",  VK_SCROLL},
        {"pause",       VK_PAUSE},
        {"numlock",     VK_NUMLOCK},
    };

    std::vector<int> Result;
    auto Parts = InString.split('+', Qt::SkipEmptyParts);

    for (auto& Part : Parts) 
    {
        auto Key = Part.trimmed().toLower();
        if (auto It = KeyMap.find(Key); It != KeyMap.end()) 
        {
            Result.push_back(It->second);
        } 
        else if (Key.length() == 1) 
        {
            // Single character — use VkKeyScan
            auto Ch = Key[0].unicode();
            SHORT VK = VkKeyScanW(Ch);
            if (VK != -1) 
            {
                Result.push_back(VK & 0xFF);
            }
        }
    }

    return Result;
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
