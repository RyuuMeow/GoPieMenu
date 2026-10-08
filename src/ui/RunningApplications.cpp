#include "RunningApplications.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <windows.h>

namespace gpm {
QStringList RunningApplications() {
    QStringList result;
    EnumWindows([](HWND window, LPARAM data) -> BOOL {
        if (!IsWindowVisible(window) || (GetWindowLongPtrW(window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) return TRUE;
        DWORD pid = 0; GetWindowThreadProcessId(window, &pid);
        if (!pid || pid == DWORD(QCoreApplication::applicationPid())) return TRUE;
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!process) return TRUE;
        wchar_t path[32768]{}; DWORD size = 32768;
        if (QueryFullProcessImageNameW(process, 0, path, &size)) {
            const auto name = QFileInfo(QString::fromWCharArray(path, int(size))).fileName();
            auto& names = *reinterpret_cast<QStringList*>(data);
            if (name.compare("explorer.exe", Qt::CaseInsensitive) && !names.contains(name, Qt::CaseInsensitive)) names.append(name);
        }
        CloseHandle(process); return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    result.sort(Qt::CaseInsensitive); return result;
}
}
