#pragma once

// =============================================================================
// GoPieMenu - macOS virtual key mapping
// =============================================================================
//
// The configuration format, the action syntax, and InputState all keep using
// Windows virtual-key codes on every platform. macOS reports physical CGKeyCode
// values instead, so this portable translation layer sits between the two.
//
// These files deliberately contain no Apple headers and no macOS-only constants:
// gpm_core builds them on Windows too (MSVC), which keeps CoreTests able to
// verify the mapping on both CI platforms.

#include <QString>

#include <vector>

namespace gpm
{

// One physical key that exists on both platforms. MacKeyCode is the macOS
// CGKeyCode (Carbon virtual key code); WindowsVk is the shared Windows VK.
struct MacKeyMapping
{
    int MacKeyCode;
    int WindowsVk;
};

// Every mapped physical key, ordered by macOS key code. Keys that share a
// Windows virtual key (the main Return key and keypad Enter) both appear here;
// the first entry for a Windows VK is the canonical key used for synthetic
// input.
[[nodiscard]] const std::vector<MacKeyMapping>& MacKeyMappings();

// Returns -1 when the key code is not part of the table.
[[nodiscard]] int WindowsVkFromMacKeyCode(int macKeyCode);

// Returns -1 when Windows has a virtual key without a physical macOS
// counterpart (Print Screen, Num Lock, Scroll Lock, Pause, F21-F24, ...).
// Generic modifier virtual keys resolve to their left-hand physical key.
[[nodiscard]] int MacKeyCodeFromWindowsVk(int windowsVk);

// English key name in the style of Windows GetKeyNameText(), localized through
// the translator. Returns an empty string for unknown virtual keys.
[[nodiscard]] QString MacKeyDisplayName(int windowsVk);

} // namespace gpm
