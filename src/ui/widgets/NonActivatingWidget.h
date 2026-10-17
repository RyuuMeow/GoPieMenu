#pragma once
#include <QWidget>
#include <QEvent>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace gpm {
// Mouse input is accepted without moving keyboard focus away from the working app.
class NonActivatingWidget : public QWidget {
public:
    explicit NonActivatingWidget(QWidget* parent = nullptr)
        : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus) {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setFocusPolicy(Qt::NoFocus);
    }
protected:
    bool event(QEvent* event) override {
        const bool handled = QWidget::event(event);
#ifdef Q_OS_WIN
        if (event->type() == QEvent::WinIdChange && internalWinId()) {
            // WM_MOUSEACTIVATE alone is too late to preserve the other process's
            // foreground window on every input path. Apply the native contract
            // whenever Qt creates/recreates the HWND, before it becomes visible.
            const auto window = reinterpret_cast<HWND>(internalWinId());
            SetWindowLongPtrW(window, GWL_EXSTYLE, GetWindowLongPtrW(window, GWL_EXSTYLE) | WS_EX_NOACTIVATE);
        }
#endif
        return handled;
    }
    bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override {
#ifdef Q_OS_WIN
        const auto id = static_cast<MSG*>(message)->message;
        if (id == WM_MOUSEACTIVATE) {
            *result = MA_NOACTIVATE;
            return true;
        }
        if (id == WM_POINTERACTIVATE) {
            *result = PA_NOACTIVATE;
            return true;
        }
#endif
        return QWidget::nativeEvent(type, message, result);
    }
};
}
