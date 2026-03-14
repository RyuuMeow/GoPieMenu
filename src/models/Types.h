#pragma once

// =============================================================================
// GoPieMenu - Core Type Definitions
// =============================================================================

#include <QString>
#include <QColor>
#include <QKeySequence>
#include <QUuid>

#include <cstdint>
#include <vector>
#include <string>

namespace gpm 
{

/**
 * Extensible action types.
 */
enum class ActionType : uint8_t 
{
    None = 0,
    LaunchApp,      // Launch an executable
    RunCommand,     // Execute a shell command
    SendHotkey,     // Simulate keyboard shortcut
    OpenFile,       // Open file/folder with default app
    OpenURL,        // Open URL in default browser
    ListMenu,       // Open a sub-list menu
};

/**
 * Mouse Buttons.
 */
enum class MouseButton : uint8_t 
{
    None = 0,
    Left,
    Right,
    Middle,
    X1,
    X2,
};

/**
 * Modifier Keys (bitmask).
 */
enum class ModifierKey : uint16_t 
{
    None    = 0x0000,
    Ctrl    = 0x0001,
    Shift   = 0x0002,
    Alt     = 0x0004,
    Win     = 0x0008,
};

// Bitwise operators for ModifierKey
inline ModifierKey operator|(ModifierKey A, ModifierKey B) 
{
    return static_cast<ModifierKey>(
        static_cast<uint16_t>(A) | static_cast<uint16_t>(B));
}

inline ModifierKey operator&(ModifierKey A, ModifierKey B) 
{
    return static_cast<ModifierKey>(
        static_cast<uint16_t>(A) & static_cast<uint16_t>(B));
}

inline bool HasModifier(ModifierKey Flags, ModifierKey Test) 
{
    return (Flags & Test) == Test;
}

/**
 * Activation Modes.
 */
enum class ActivationMode : uint8_t 
{
    MouseHold = 0, // Default: Hold right click, move, release right click
    KeyHold,       // Hold custom key, move, release custom key
    KeyToggle,     // Press custom key, move, click left mouse to execute
};

/**
 * Trigger Definition.
 */
struct TriggerDef 
{
    ActivationMode Mode = ActivationMode::MouseHold;
    ModifierKey    Modifiers = ModifierKey::None;
    MouseButton    Button = MouseButton::Right;
    uint32_t       VKCode = 0; // Virtual Key Code for KeyHold/KeyToggle

    [[nodiscard]] bool IsValid() const 
    {
        if (Mode == ActivationMode::MouseHold) 
        {
            return Button != MouseButton::None;
        }
        return VKCode != 0;
    }

    bool operator==(const TriggerDef&) const = default;
};

// === String conversion helpers ===

[[nodiscard]] inline QString ActivationModeToString(ActivationMode Mode) 
{
    switch (Mode) 
    {
        case ActivationMode::MouseHold: return QStringLiteral("MouseHold");
        case ActivationMode::KeyHold:   return QStringLiteral("KeyHold");
        case ActivationMode::KeyToggle: return QStringLiteral("KeyToggle");
        default:                        return QStringLiteral("Unknown");
    }
}

[[nodiscard]] inline ActivationMode StringToActivationMode(const QString& Str) 
{
    if (Str == "MouseHold") return ActivationMode::MouseHold;
    if (Str == "KeyHold")   return ActivationMode::KeyHold;
    if (Str == "KeyToggle") return ActivationMode::KeyToggle;
    return ActivationMode::MouseHold;
}

[[nodiscard]] inline QString ActionTypeToString(ActionType Type) 
{
    switch (Type) 
    {
        case ActionType::LaunchApp:   return QStringLiteral("LaunchApp");
        case ActionType::RunCommand:  return QStringLiteral("RunCommand");
        case ActionType::SendHotkey:  return QStringLiteral("SendHotkey");
        case ActionType::OpenFile:    return QStringLiteral("OpenFile");
        case ActionType::OpenURL:     return QStringLiteral("OpenURL");
        case ActionType::ListMenu:    return QStringLiteral("ListMenu");
        default:                      return QStringLiteral("None");
    }
}

[[nodiscard]] inline ActionType StringToActionType(const QString& Str) 
{
    if (Str == "LaunchApp")   return ActionType::LaunchApp;
    if (Str == "RunCommand")  return ActionType::RunCommand;
    if (Str == "SendHotkey")  return ActionType::SendHotkey;
    if (Str == "OpenFile")    return ActionType::OpenFile;
    if (Str == "OpenURL")     return ActionType::OpenURL;
    if (Str == "ListMenu")    return ActionType::ListMenu;
    return ActionType::None;
}

[[nodiscard]] inline QString MouseButtonToString(MouseButton Btn) 
{
    switch (Btn) 
    {
        case MouseButton::Left:   return QStringLiteral("Left");
        case MouseButton::Right:  return QStringLiteral("Right");
        case MouseButton::Middle: return QStringLiteral("Middle");
        case MouseButton::X1:    return QStringLiteral("X1");
        case MouseButton::X2:    return QStringLiteral("X2");
        default:                  return QStringLiteral("None");
    }
}

[[nodiscard]] inline MouseButton StringToMouseButton(const QString& Str) 
{
    if (Str == "Left")   return MouseButton::Left;
    if (Str == "Right")  return MouseButton::Right;
    if (Str == "Middle") return MouseButton::Middle;
    if (Str == "X1")    return MouseButton::X1;
    if (Str == "X2")    return MouseButton::X2;
    return MouseButton::None;
}

[[nodiscard]] inline uint16_t ModifierKeyToUint(ModifierKey Mod) 
{
    return static_cast<uint16_t>(Mod);
}

[[nodiscard]] inline ModifierKey UintToModifierKey(uint16_t Val) 
{
    return static_cast<ModifierKey>(Val);
}

[[nodiscard]] inline QString ModifierKeyToString(ModifierKey Mod) 
{
    QStringList Parts;
    if (HasModifier(Mod, ModifierKey::Ctrl))  Parts << QStringLiteral("Ctrl");
    if (HasModifier(Mod, ModifierKey::Shift)) Parts << QStringLiteral("Shift");
    if (HasModifier(Mod, ModifierKey::Alt))   Parts << QStringLiteral("Alt");
    if (HasModifier(Mod, ModifierKey::Win))   Parts << QStringLiteral("Win");
    return Parts.join('+');
}

} // namespace gpm
