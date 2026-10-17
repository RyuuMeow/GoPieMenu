#pragma once
#include <QWidget>
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
    bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override {
#ifdef Q_OS_WIN
        if (static_cast<MSG*>(message)->message == WM_MOUSEACTIVATE) {
            *result = MA_NOACTIVATE;
            return true;
        }
#endif
        return QWidget::nativeEvent(type, message, result);
    }
};
}
