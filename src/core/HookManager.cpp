// =============================================================================
// GoPieMenu - HookManager Implementation
// =============================================================================

#include "HookManager.h"

#include <QCoreApplication>
#include <QCursor>
#include <QFileInfo>
#include <QDebug>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

namespace gpm 
{

HookManager* HookManager::Instance = nullptr;

HookManager::HookManager(ConfigManager* InConfigMgr, QObject* Parent)
    : IHookProvider(Parent)
    , ConfigMgr(InConfigMgr)
{
    Instance = this;
}

HookManager::~HookManager()
{
    Uninstall();
    if (Instance == this) 
    {
        Instance = nullptr;
    }
}

bool HookManager::Install()
{
    if (KBHook && MSHook) 
    {
        return true;
    }

    HINSTANCE HInst = GetModuleHandle(nullptr);

    KBHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, HInst, 0);
    if (!KBHook) 
    {
        qWarning() << "[HookManager] Failed to install keyboard hook. Error:" << GetLastError();
        return false;
    }

    MSHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, HInst, 0);
    if (!MSHook) 
    {
        qWarning() << "[HookManager] Failed to install mouse hook. Error:" << GetLastError();
        UnhookWindowsHookEx(KBHook);
        KBHook = nullptr;
        return false;
    }

    qDebug() << "[HookManager] Hooks installed successfully.";
    return true;
}

void HookManager::Uninstall()
{
    if (KBHook) 
    {
        UnhookWindowsHookEx(KBHook);
        KBHook = nullptr;
    }
    if (MSHook) 
    {
        UnhookWindowsHookEx(MSHook);
        MSHook = nullptr;
    }
    bTriggerActive = false;
    qDebug() << "[HookManager] Hooks uninstalled.";
}

bool HookManager::IsInstalled() const
{
    return KBHook != nullptr && MSHook != nullptr;
}

// === Static Callbacks ===

LRESULT CALLBACK HookManager::KeyboardProc(int NCode, WPARAM WParam, LPARAM LParam)
{
    if (NCode >= 0 && Instance) 
    {
        auto* KB = reinterpret_cast<KBDLLHOOKSTRUCT*>(LParam);
        if (Instance->OnKeyEvent(WParam, *KB)) 
        {
            return 1; // Swallow event
        }
    }
    return CallNextHookEx(nullptr, NCode, WParam, LParam);
}

LRESULT CALLBACK HookManager::MouseProc(int NCode, WPARAM WParam, LPARAM LParam)
{
    if (NCode >= 0 && Instance) 
    {
        auto* MS = reinterpret_cast<MSLLHOOKSTRUCT*>(LParam);
        if (Instance->OnMouseEvent(WParam, *MS)) 
        {
            return 1; // Swallow event
        }
    }
    return CallNextHookEx(nullptr, NCode, WParam, LParam);
}

// === Key Event Handling ===

bool HookManager::OnKeyEvent(WPARAM WParam, const KBDLLHOOKSTRUCT& KB)
{
    bool bPressed  = (WParam == WM_KEYDOWN || WParam == WM_SYSKEYDOWN);
    bool bReleased = (WParam == WM_KEYUP || WParam == WM_SYSKEYUP);
    
    UpdateModifierState(KB.vkCode, bPressed);

    // Filter out lone modifier keys
    bool bIsOSModifier = false;
    switch (KB.vkCode) 
    {
        case VK_LWIN: case VK_RWIN:
        case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT:
        case VK_MENU: case VK_LMENU: case VK_RMENU:
            bIsOSModifier = true; break;
    }

    if (bTriggerActive) 
    {
        if (bIsOSModifier) 
        {
            return false; // NEVER swallow OS modifiers
        }
        
        if (KB.vkCode == VK_ESCAPE && bPressed) 
        {
            bTriggerActive = false;
            ActiveProfileId.clear();
            emit CancelRequested();
            return true;
        }

        auto* Profile = ConfigMgr->FindProfile(ActiveProfileId);
        if (!Profile) 
        {
            return true; // Swallow while active but broken
        }

        if (Profile->Trigger.Mode == ActivationMode::KeyHold && bReleased) 
        {
            if (Profile->Trigger.VKCode == KB.vkCode) 
            {
                // KeyHold released -> execute
                bTriggerActive = false;
                ActiveProfileId.clear();
                emit TriggerReleased(QCursor::pos());
                return true;
            }
        }

        if (Profile->Trigger.Mode == ActivationMode::KeyToggle && bPressed) 
        {
            if (Profile->Trigger.VKCode == KB.vkCode && Profile->Trigger.Modifiers == GetCurrentModifiers()) 
            {
                // Toggle pressed again -> cancel
                bTriggerActive = false;
                ActiveProfileId.clear();
                emit CancelRequested();
                return true;
            }
        }
        
        // Swallow everything else while trigger is active to prevent OS from registering typing
        return true; 
    }

    // Try to trigger on key press (if not a modifier)
    if (bPressed && !bIsOSModifier && !bTriggerActive) 
    {
        if (auto* Profile = FindMatchingProfile(MouseButton::None, KB.vkCode)) 
        {
            bTriggerActive   = true;
            ActiveProfileId = Profile->Id;
            emit Triggered(Profile->Id, QCursor::pos());
            return true; // Swallow the activating keystroke
        }
    }

    return false;
}

void HookManager::UpdateModifierState(DWORD VKCode, bool bPressed)
{
    switch (VKCode) 
    {
        case VK_LCONTROL: case VK_RCONTROL: case VK_CONTROL:
            bCtrlDown = bPressed; break;
        case VK_LSHIFT: case VK_RSHIFT: case VK_SHIFT:
            bShiftDown = bPressed; break;
        case VK_LMENU: case VK_RMENU: case VK_MENU:
            bAltDown = bPressed; break;
        case VK_LWIN: case VK_RWIN:
            bWinDown = bPressed; break;
        default: break;
    }
}

ModifierKey HookManager::GetCurrentModifiers() const
{
    ModifierKey Mod = ModifierKey::None;
    if (bCtrlDown)  Mod = Mod | ModifierKey::Ctrl;
    if (bShiftDown) Mod = Mod | ModifierKey::Shift;
    if (bAltDown)   Mod = Mod | ModifierKey::Alt;
    if (bWinDown)   Mod = Mod | ModifierKey::Win;
    return Mod;
}

// === Mouse Event Handling ===

bool HookManager::OnMouseEvent(WPARAM WParam, const MSLLHOOKSTRUCT& MS)
{
    QPoint Pos = QCursor::pos();

    if (WParam == WM_MOUSEMOVE) 
    {
        if (bTriggerActive) 
        {
            emit MouseMoved(Pos);
        }
        return false; // Never swallow mouse move
    }

    auto* Profile = bTriggerActive ? ConfigMgr->FindProfile(ActiveProfileId) : nullptr;

    // Determine which mouse button
    MouseButton Btn = MouseButton::None;
    bool bIsDown = false;
    bool bIsLeftClick = false;

    switch (WParam) 
    {
        case WM_RBUTTONDOWN: Btn = MouseButton::Right;  bIsDown = true;  break;
        case WM_RBUTTONUP:   Btn = MouseButton::Right;  bIsDown = false; break;
        case WM_MBUTTONDOWN: Btn = MouseButton::Middle; bIsDown = true;  break;
        case WM_MBUTTONUP:   Btn = MouseButton::Middle; bIsDown = false; break;
        case WM_XBUTTONDOWN: Btn = (HIWORD(MS.mouseData) == XBUTTON1) ? MouseButton::X1 : MouseButton::X2; bIsDown = true;  break;
        case WM_XBUTTONUP:   Btn = (HIWORD(MS.mouseData) == XBUTTON1) ? MouseButton::X1 : MouseButton::X2; bIsDown = false; break;
        case WM_LBUTTONDOWN: bIsLeftClick = true; bIsDown = true; break;
        case WM_LBUTTONUP:   bIsLeftClick = true; bIsDown = false; break;
    }

    if (bTriggerActive && Profile) 
    {
        // Handle KeyToggle Mode
        if (Profile->Trigger.Mode == ActivationMode::KeyToggle) 
        {
            if (bIsLeftClick && bIsDown) 
            { // Left click to execute in toggle mode
                bTriggerActive = false;
                ActiveProfileId.clear();
                emit TriggerReleased(Pos);
                return true; 
            }
            return true; // Swallow all other clicks
        }
        
        // Handle MouseHold Mode
        if (Profile->Trigger.Mode == ActivationMode::MouseHold) 
        {
            if (!bIsDown && Profile->Trigger.Button == Btn) 
            {
                // Correct button released -> execute
                bTriggerActive = false;
                ActiveProfileId.clear();
                emit TriggerReleased(Pos);
                return true;
            }
            return true; // Swallow all other clicks
        }
        
        // Handle KeyHold Mode -> Swallow all mouse clicks while Key is held down
        if (Profile->Trigger.Mode == ActivationMode::KeyHold) 
        {
            return true;
        }

        return true; // Fallback swallow
    }

    // Try to activate via mouse
    if (bIsDown && !bTriggerActive && Btn != MouseButton::None) 
    {
        if (auto* NewProfile = FindMatchingProfile(Btn, 0)) 
        {
            bTriggerActive   = true;
            ActiveProfileId = NewProfile->Id;
            emit Triggered(NewProfile->Id, Pos);
            return true; // Swallow activating click
        }
    }

    return bTriggerActive;
}

// === Profile Matching ===

const PieMenuConfig* HookManager::FindMatchingProfile(MouseButton Btn, uint32_t VKCode) const
{
    auto Mods = GetCurrentModifiers();
    const PieMenuConfig* BestMatch = nullptr;
    int BestScore = -1;

    for (const auto& Profile : ConfigMgr->GetConfig().Profiles) 
    {
        if (!Profile.bEnabled) 
        {
            continue;
        }
        if (Profile.Trigger.Modifiers != Mods) 
        {
            continue;
        }
        
        if (Profile.Trigger.Mode == ActivationMode::MouseHold) 
        {
            if (Profile.Trigger.Button != Btn) 
            {
                continue;
            }
        } 
        else 
        {
            // KeyHold or KeyToggle
            if (Profile.Trigger.VKCode != VKCode || VKCode == 0) 
            {
                continue;
            }
        }
        
        // Score: 1 for exact App Scope match, 0 for Global (empty list)
        int Score = -1;
        if (Profile.AppFilter.isEmpty()) 
        {
            Score = 0; // Global
        } 
        else if (MatchesAppFilter(Profile)) 
        {
            Score = 1; // Exact match
        }
        
        if (Score > BestScore) 
        {
            BestScore = Score;
            BestMatch = &Profile;
        }
    }
    return BestMatch;
}

bool HookManager::MatchesAppFilter(const PieMenuConfig& Profile) const
{
    if (Profile.AppFilter.isEmpty()) 
    {
        return true; // Global
    }

    auto ProcName = GetForegroundProcessName();
    if (ProcName.isEmpty()) 
    {
        return true;
    }

    for (const auto& Filter : Profile.AppFilter) 
    {
        if (ProcName.compare(Filter, Qt::CaseInsensitive) == 0) 
        {
            return true;
        }
    }
    return false;
}

QString HookManager::GetForegroundProcessName() const
{
    HWND Hwnd = GetForegroundWindow();
    if (!Hwnd) 
    {
        return {};
    }

    DWORD Pid = 0;
    GetWindowThreadProcessId(Hwnd, &Pid);
    if (Pid == 0) 
    {
        return {};
    }

    HANDLE HProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid);
    if (!HProc) 
    {
        return {};
    }

    wchar_t Path[MAX_PATH] = {};
    DWORD PathLen = MAX_PATH;
    QueryFullProcessImageNameW(HProc, 0, Path, &PathLen);
    CloseHandle(HProc);

    return QFileInfo(QString::fromWCharArray(Path, PathLen)).fileName();
}

} // namespace gpm
