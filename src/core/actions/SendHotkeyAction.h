#pragma once

// =============================================================================
// GoPieMenu - SendHotkeyAction
// =============================================================================

#include "core/IActionHandler.h"

#include <QDebug>
#include <QStringList>

#include <vector>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace gpm 
{

/**
 * Simulates keyboard shortcuts. Windows uses Win32 SendInput; macOS posts
 * Quartz keyboard events. Both keep the same semantics: release modifiers that
 * are physically held, send the chord, release it, then press the physical
 * modifiers back so the system state matches the real keyboard.
 */
class SendHotkeyAction : public IActionHandler 
{
public:
    // === IActionHandler Implementation ===
    
    [[nodiscard]] virtual ActionType GetSupportedType() const override 
    { 
        return ActionType::SendHotkey; 
    }
    
    [[nodiscard]] virtual QString GetDisplayName() const override 
    { 
        return QStringLiteral("Send Hotkey"); 
    }
    
    [[nodiscard]] virtual QString GetDescription() const override 
    {
        return QStringLiteral("Simulate a keyboard shortcut. actionData = key combo like 'Ctrl+Shift+A'.");
    }

    virtual bool Execute(const PieItem& Item) override;

private:
    [[nodiscard]] static std::vector<int> GetHeldModifiers();
#ifdef Q_OS_WIN
    [[nodiscard]] static bool bIsExtendedKey(int VK);
#endif
};

} // namespace gpm
