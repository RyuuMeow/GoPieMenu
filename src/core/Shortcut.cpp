#include "Shortcut.h"
#include <QHash>
#include <QStringList>
#include <algorithm>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace gpm {
std::vector<int> ParseShortcut(const QString& text) {
    static const QHash<QString, int> named{
        {"tab",0x09}, {"enter",0x0d}, {"return",0x0d}, {"escape",0x1b}, {"esc",0x1b},
        {"space",0x20}, {"backspace",0x08}, {"delete",0x2e}, {"del",0x2e},
        {"insert",0x2d}, {"ins",0x2d}, {"home",0x24}, {"end",0x23},
        {"pageup",0x21}, {"pgup",0x21}, {"pagedown",0x22}, {"pgdown",0x22}, {"pgdn",0x22},
        {"up",0x26}, {"down",0x28}, {"left",0x25}, {"right",0x27},
        {"print",0x2c}, {"printscreen",0x2c}, {"prtsc",0x2c}, {"scrolllock",0x91},
        {"pause",0x13}, {"numlock",0x90}, {"capslock",0x14}, {"menu",0x5d}
    };
    auto input = text.trimmed().toLower();
    if (input.endsWith('+')) input = input.left(input.size()-1) + "plus";
    bool ctrl = false, shift = false, alt = false, win = false;
    int mainKey = 0;
    for (auto part : input.split('+')) {
        part = part.trimmed();
        if (part.isEmpty()) return {};
        if (part == "ctrl" || part == "control") { ctrl = true; continue; }
        if (part == "shift") { shift = true; continue; }
        if (part == "alt") { alt = true; continue; }
        if (part == "win" || part == "meta") { win = true; continue; }
        if (mainKey) return {}; // A second ordinary key is not a single shortcut.
        mainKey = named.value(part);
        if (!mainKey && part.startsWith('f')) {
            bool ok = false; const int number = part.mid(1).toInt(&ok);
            if (ok && number >= 1 && number <= 24) mainKey = 0x70 + number - 1;
        }
        if (part == "plus") part = "+";
        if (!mainKey && part.size() == 1) {
#ifdef Q_OS_WIN
            const auto code = VkKeyScanW(part[0].unicode());
            if (code == -1) return {};
            mainKey = code & 0xff;
            shift |= (code & 0x100) != 0; ctrl |= (code & 0x200) != 0; alt |= (code & 0x400) != 0;
#else
            const auto c = part[0].toUpper().unicode();
            if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) mainKey = c;
#endif
        }
        if (!mainKey) return {};
    }
    if (!mainKey) return {};
    std::vector<int> keys;
    if (ctrl) keys.push_back(0xa2);
    if (shift) keys.push_back(0xa0);
    if (alt) keys.push_back(0xa4);
    if (win) keys.push_back(0x5b);
    keys.push_back(mainKey);
    return keys;
}
}
