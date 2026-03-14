#pragma once

// =============================================================================
// GoPieMenu - SendHotkeyAction
// =============================================================================

#include "core/IActionHandler.h"

#include <QDebug>
#include <QStringList>

#include <windows.h>
#include <vector>
#include <unordered_map>

namespace gpm 
{

/**
 * Simulates keyboard shortcuts using Win32 SendInput.
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
    [[nodiscard]] static std::vector<int> ParseHotkey(const QString& InString);
    [[nodiscard]] static bool bIsExtendedKey(int VK);
};

} // namespace gpm
