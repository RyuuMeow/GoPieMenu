#include "platform/mac/MacKeyMap.h"

#include <QCoreApplication>

#include <vector>

namespace gpm
{
namespace
{
// Windows virtual-key codes used by the shared configuration and InputState.
// Spelled out so this file never depends on <windows.h>.
constexpr int VkBack = 0x08;
constexpr int VkTab = 0x09;
constexpr int VkClear = 0x0C;
constexpr int VkReturn = 0x0D;
constexpr int VkShift = 0x10;
constexpr int VkControl = 0x11;
constexpr int VkAlt = 0x12;
constexpr int VkPause = 0x13;
constexpr int VkCapsLock = 0x14;
constexpr int VkEscape = 0x1B;
constexpr int VkSpace = 0x20;
constexpr int VkPageUp = 0x21;
constexpr int VkPageDown = 0x22;
constexpr int VkEnd = 0x23;
constexpr int VkHome = 0x24;
constexpr int VkLeft = 0x25;
constexpr int VkUp = 0x26;
constexpr int VkRight = 0x27;
constexpr int VkDown = 0x28;
constexpr int VkPrintScreen = 0x2C;
constexpr int VkInsert = 0x2D;
constexpr int VkDelete = 0x2E;
constexpr int VkHelp = 0x2F;
constexpr int VkApps = 0x5D;
constexpr int VkNumPad0 = 0x60;
constexpr int VkNumLock = 0x90;
constexpr int VkScrollLock = 0x91;
constexpr int VkMultiply = 0x6A;
constexpr int VkAdd = 0x6B;
constexpr int VkSubtract = 0x6D;
constexpr int VkDecimal = 0x6E;
constexpr int VkDivide = 0x6F;
constexpr int VkLShift = 0xA0;
constexpr int VkRShift = 0xA1;
constexpr int VkLControl = 0xA2;
constexpr int VkRControl = 0xA3;
constexpr int VkLAlt = 0xA4;
constexpr int VkRAlt = 0xA5;
constexpr int VkVolumeMute = 0xAD;
constexpr int VkVolumeDown = 0xAE;
constexpr int VkVolumeUp = 0xAF;
constexpr int VkOemSemicolon = 0xBA;
constexpr int VkOemPlus = 0xBB;
constexpr int VkOemComma = 0xBC;
constexpr int VkOemMinus = 0xBD;
constexpr int VkOemPeriod = 0xBE;
constexpr int VkOemQuestion = 0xBF;
constexpr int VkOemTilde = 0xC0;
constexpr int VkOemOpenBracket = 0xDB;
constexpr int VkOemPipe = 0xDC;
constexpr int VkOemCloseBracket = 0xDD;
constexpr int VkOemQuote = 0xDE;
constexpr int VkLWin = 0x5B;
constexpr int VkRWin = 0x5C;

// macOS virtual key codes, numbered exactly as the Carbon Events.h constants
// (kVK_ANSI_A = 0x00 and so on). The comments name the constant so the table can
// be checked against Apple's headers without including them here.
struct MacRow
{
    int MacKeyCode;
    int WindowsVk;
};

// Canonical table in macOS key-code order. Aliases (main Return and keypad
// Enter) keep both physical codes, which makes recording unambiguous while
// still mapping back to the canonical Return code.
const MacRow Rows[] = {
    {0x00, 'A'},                 // kVK_ANSI_A
    {0x01, 'S'},                 // kVK_ANSI_S
    {0x02, 'D'},                 // kVK_ANSI_D
    {0x03, 'F'},                 // kVK_ANSI_F
    {0x04, 'H'},                 // kVK_ANSI_H
    {0x05, 'G'},                 // kVK_ANSI_G
    {0x06, 'Z'},                 // kVK_ANSI_Z
    {0x07, 'X'},                 // kVK_ANSI_X
    {0x08, 'C'},                 // kVK_ANSI_C
    {0x09, 'V'},                 // kVK_ANSI_V
    {0x0B, 'B'},                 // kVK_ANSI_B
    {0x0C, 'Q'},                 // kVK_ANSI_Q
    {0x0D, 'W'},                 // kVK_ANSI_W
    {0x0E, 'E'},                 // kVK_ANSI_E
    {0x0F, 'R'},                 // kVK_ANSI_R
    {0x10, 'Y'},                 // kVK_ANSI_Y
    {0x11, 'T'},                 // kVK_ANSI_T
    {0x12, '1'},                 // kVK_ANSI_1
    {0x13, '2'},                 // kVK_ANSI_2
    {0x14, '3'},                 // kVK_ANSI_3
    {0x15, '4'},                 // kVK_ANSI_4
    {0x16, '6'},                 // kVK_ANSI_6
    {0x17, '5'},                 // kVK_ANSI_5
    {0x18, VkOemPlus},           // kVK_ANSI_Equal
    {0x19, '9'},                 // kVK_ANSI_9
    {0x1A, '7'},                 // kVK_ANSI_7
    {0x1B, VkOemMinus},          // kVK_ANSI_Minus
    {0x1C, '8'},                 // kVK_ANSI_8
    {0x1D, '0'},                 // kVK_ANSI_0
    {0x1E, VkOemCloseBracket},   // kVK_ANSI_RightBracket
    {0x1F, 'O'},                 // kVK_ANSI_O
    {0x20, 'U'},                 // kVK_ANSI_U
    {0x21, VkOemOpenBracket},    // kVK_ANSI_LeftBracket
    {0x22, 'I'},                 // kVK_ANSI_I
    {0x23, 'P'},                 // kVK_ANSI_P
    {0x24, VkReturn},            // kVK_Return
    {0x25, 'L'},                 // kVK_ANSI_L
    {0x26, 'J'},                 // kVK_ANSI_J
    {0x27, VkOemQuote},          // kVK_ANSI_Quote
    {0x28, 'K'},                 // kVK_ANSI_K
    {0x29, VkOemSemicolon},      // kVK_ANSI_Semicolon
    {0x2A, VkOemPipe},           // kVK_ANSI_Backslash
    {0x2B, VkOemComma},          // kVK_ANSI_Comma
    {0x2C, VkOemQuestion},       // kVK_ANSI_Slash
    {0x2D, 'N'},                 // kVK_ANSI_N
    {0x2E, 'M'},                 // kVK_ANSI_M
    {0x2F, VkOemPeriod},         // kVK_ANSI_Period
    {0x30, VkTab},               // kVK_Tab
    {0x31, VkSpace},             // kVK_Space
    {0x32, VkOemTilde},          // kVK_ANSI_Grave
    {0x33, VkBack},              // kVK_Delete (backspace)
    {0x35, VkEscape},            // kVK_Escape
    {0x36, VkRWin},              // kVK_RightCommand
    {0x37, VkLWin},              // kVK_Command
    {0x38, VkLShift},            // kVK_Shift
    {0x39, VkCapsLock},          // kVK_CapsLock
    {0x3A, VkLAlt},              // kVK_Option
    {0x3B, VkLControl},          // kVK_Control
    {0x3C, VkRShift},            // kVK_RightShift
    {0x3D, VkRAlt},              // kVK_RightOption
    {0x3E, VkRControl},          // kVK_RightControl
    {0x40, 0x80},                // kVK_F17
    {0x41, VkDecimal},           // kVK_ANSI_KeypadDecimal
    {0x43, VkMultiply},          // kVK_ANSI_KeypadMultiply
    {0x45, VkAdd},               // kVK_ANSI_KeypadPlus
    {0x47, VkClear},             // kVK_ANSI_KeypadClear
    {0x48, VkVolumeUp},          // kVK_VolumeUp
    {0x49, VkVolumeDown},        // kVK_VolumeDown
    {0x4A, VkVolumeMute},        // kVK_Mute
    {0x4B, VkDivide},            // kVK_ANSI_KeypadDivide
    {0x4C, VkReturn},            // kVK_ANSI_KeypadEnter
    {0x4E, VkSubtract},          // kVK_ANSI_KeypadMinus
    {0x4F, 0x81},                // kVK_F18
    {0x50, 0x82},                // kVK_F19
    {0x52, VkNumPad0},           // kVK_ANSI_Keypad0
    {0x53, 0x61},                // kVK_ANSI_Keypad1
    {0x54, 0x62},                // kVK_ANSI_Keypad2
    {0x55, 0x63},                // kVK_ANSI_Keypad3
    {0x56, 0x64},                // kVK_ANSI_Keypad4
    {0x57, 0x65},                // kVK_ANSI_Keypad5
    {0x58, 0x66},                // kVK_ANSI_Keypad6
    {0x59, 0x67},                // kVK_ANSI_Keypad7
    {0x5A, 0x83},                // kVK_F20
    {0x5B, 0x68},                // kVK_ANSI_Keypad8
    {0x5C, 0x69},                // kVK_ANSI_Keypad9
    {0x60, 0x74},                // kVK_F5
    {0x61, 0x75},                // kVK_F6
    {0x62, 0x76},                // kVK_F7
    {0x63, 0x72},                // kVK_F3
    {0x64, 0x77},                // kVK_F8
    {0x65, 0x78},                // kVK_F9
    {0x67, 0x7A},                // kVK_F11
    {0x69, 0x7C},                // kVK_F13
    {0x6A, 0x7F},                // kVK_F16
    {0x6B, 0x7D},                // kVK_F14
    {0x6D, 0x79},                // kVK_F10
    {0x6E, VkApps},              // kVK_ContextualMenu
    {0x6F, 0x7B},                // kVK_F12
    {0x71, 0x7E},                // kVK_F15
    {0x72, VkHelp},              // kVK_Help
    {0x73, VkHome},              // kVK_Home
    {0x74, VkPageUp},            // kVK_PageUp
    {0x75, VkDelete},            // kVK_ForwardDelete
    {0x76, 0x73},                // kVK_F4
    {0x77, VkEnd},               // kVK_End
    {0x78, 0x71},                // kVK_F2
    {0x79, VkPageDown},          // kVK_PageDown
    {0x7A, 0x70},                // kVK_F1
    {0x7B, VkLeft},              // kVK_LeftArrow
    {0x7C, VkRight},             // kVK_RightArrow
    {0x7D, VkDown},              // kVK_DownArrow
    {0x7E, VkUp},                // kVK_UpArrow
};

} // namespace

const std::vector<MacKeyMapping>& MacKeyMappings()
{
    static const std::vector<MacKeyMapping> mappings = [] {
        std::vector<MacKeyMapping> result;
        result.reserve(sizeof(Rows) / sizeof(Rows[0]));
        for (const auto& row : Rows) result.push_back({row.MacKeyCode, row.WindowsVk});
        return result;
    }();
    return mappings;
}

int WindowsVkFromMacKeyCode(int macKeyCode)
{
    for (const auto& row : Rows)
    {
        if (row.MacKeyCode == macKeyCode) return row.WindowsVk;
    }
    return -1;
}

int MacKeyCodeFromWindowsVk(int windowsVk)
{
    // Generic modifiers keep the shared semantics of any-side state; synthetic
    // events use the left-hand physical key, matching the sided VKs that
    // ParseShortcut emits (LCtrl, LShift, LAlt, LWin).
    switch (windowsVk)
    {
        case VkShift:   return 0x38;
        case VkControl: return 0x3B;
        case VkAlt:     return 0x3A;
        default:        break;
    }
    for (const auto& row : Rows)
    {
        if (row.WindowsVk == windowsVk) return row.MacKeyCode;
    }
    return -1;
}

QString MacKeyDisplayName(int windowsVk)
{
    if (windowsVk <= 0 || windowsVk > 0xFF) return {};

    // Letters and digits are their own names (the recorder stores ASCII VKs).
    if ((windowsVk >= 'A' && windowsVk <= 'Z') || (windowsVk >= '0' && windowsVk <= '9'))
    {
        return QString(QChar(static_cast<ushort>(windowsVk)));
    }
    // F1-F24 share the configuration's virtual-key range on every platform.
    if (windowsVk >= 0x70 && windowsVk <= 0x87)
    {
        return QStringLiteral("F%1").arg(windowsVk - 0x70 + 1);
    }
    if (windowsVk >= VkNumPad0 && windowsVk <= VkNumPad0 + 9)
    {
        return QCoreApplication::translate("MacKeyMap", "Num %1").arg(windowsVk - VkNumPad0);
    }

    switch (windowsVk)
    {
        case VkBack:                 return QCoreApplication::translate("MacKeyMap", "Backspace");
        case VkTab:                  return QCoreApplication::translate("MacKeyMap", "Tab");
        case VkClear:                return QCoreApplication::translate("MacKeyMap", "Clear");
        case VkReturn:               return QCoreApplication::translate("MacKeyMap", "Enter");
        case VkShift:                return QCoreApplication::translate("MacKeyMap", "Shift");
        case VkControl:              return QCoreApplication::translate("MacKeyMap", "Ctrl");
        case VkAlt:                  return QCoreApplication::translate("MacKeyMap", "Alt");
        case VkPause:                return QCoreApplication::translate("MacKeyMap", "Pause");
        case VkNumLock:              return QCoreApplication::translate("MacKeyMap", "Num Lock");
        case VkScrollLock:           return QCoreApplication::translate("MacKeyMap", "Scroll Lock");
        case VkCapsLock:             return QCoreApplication::translate("MacKeyMap", "Caps Lock");
        case VkEscape:               return QCoreApplication::translate("MacKeyMap", "Esc");
        case VkSpace:                return QCoreApplication::translate("MacKeyMap", "Space");
        case VkPageUp:               return QCoreApplication::translate("MacKeyMap", "Page Up");
        case VkPageDown:             return QCoreApplication::translate("MacKeyMap", "Page Down");
        case VkEnd:                  return QCoreApplication::translate("MacKeyMap", "End");
        case VkHome:                 return QCoreApplication::translate("MacKeyMap", "Home");
        case VkLeft:                 return QCoreApplication::translate("MacKeyMap", "Left");
        case VkUp:                   return QCoreApplication::translate("MacKeyMap", "Up");
        case VkRight:                return QCoreApplication::translate("MacKeyMap", "Right");
        case VkDown:                 return QCoreApplication::translate("MacKeyMap", "Down");
        case VkPrintScreen:          return QCoreApplication::translate("MacKeyMap", "Print Screen");
        case VkInsert:               return QCoreApplication::translate("MacKeyMap", "Insert");
        case VkDelete:               return QCoreApplication::translate("MacKeyMap", "Delete");
        case VkHelp:                 return QCoreApplication::translate("MacKeyMap", "Help");
        case VkApps:                 return QCoreApplication::translate("MacKeyMap", "Menu");
        case VkLShift:               return QCoreApplication::translate("MacKeyMap", "Left Shift");
        case VkRShift:               return QCoreApplication::translate("MacKeyMap", "Right Shift");
        case VkLControl:             return QCoreApplication::translate("MacKeyMap", "Left Ctrl");
        case VkRControl:             return QCoreApplication::translate("MacKeyMap", "Right Ctrl");
        case VkLAlt:                 return QCoreApplication::translate("MacKeyMap", "Left Alt");
        case VkRAlt:                 return QCoreApplication::translate("MacKeyMap", "Right Alt");
        case VkLWin:                 return QCoreApplication::translate("MacKeyMap", "Left Command");
        case VkRWin:                 return QCoreApplication::translate("MacKeyMap", "Right Command");
        case VkMultiply:             return QCoreApplication::translate("MacKeyMap", "Num *");
        case VkAdd:                  return QCoreApplication::translate("MacKeyMap", "Num +");
        case VkSubtract:             return QCoreApplication::translate("MacKeyMap", "Num -");
        case VkDecimal:              return QCoreApplication::translate("MacKeyMap", "Num .");
        case VkDivide:               return QCoreApplication::translate("MacKeyMap", "Num /");
        case VkVolumeMute:           return QCoreApplication::translate("MacKeyMap", "Mute");
        case VkVolumeDown:           return QCoreApplication::translate("MacKeyMap", "Volume Down");
        case VkVolumeUp:             return QCoreApplication::translate("MacKeyMap", "Volume Up");
        case VkOemSemicolon:         return QStringLiteral(";");
        case VkOemPlus:              return QStringLiteral("=");
        case VkOemComma:             return QStringLiteral(",");
        case VkOemMinus:             return QStringLiteral("-");
        case VkOemPeriod:            return QStringLiteral(".");
        case VkOemQuestion:          return QStringLiteral("/");
        case VkOemTilde:             return QStringLiteral("`");
        case VkOemOpenBracket:       return QStringLiteral("[");
        case VkOemPipe:              return QStringLiteral("\\");
        case VkOemCloseBracket:      return QStringLiteral("]");
        case VkOemQuote:             return QStringLiteral("'");
        default:                     return {};
    }
}

} // namespace gpm
