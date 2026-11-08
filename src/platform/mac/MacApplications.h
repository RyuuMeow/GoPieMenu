#pragma once
#include <QString>
#include <QStringList>
#include <functional>

namespace gpm {
// macOS application enumeration for the editor's application scope and for the
// foreground arbitration used by persistent menus.
//
// Names are bundle executable file names (for example "Safari"), not localized
// display names, so the foreground name, the running-app list, and the stored
// app filter all compare equal. GoPieMenu itself is never reported.
QString MacForegroundProcessName();
QStringList MacRunningApplications();

// NSWorkspace application-activation observer. Hooks install one handler and
// clear it when hooks uninstall; the handler always runs on the main thread.
void MacSetForegroundChangeHandler(std::function<void()> handler);
void MacClearForegroundChangeHandler();
}
