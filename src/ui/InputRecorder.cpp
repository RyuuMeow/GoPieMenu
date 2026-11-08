#include "InputRecorder.h"
#include <QApplication>
#include <QKeyCombination>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QKeySequence>
#ifdef Q_OS_MACOS
#include "platform/mac/MacKeyMap.h"
#endif

namespace gpm {
#ifdef Q_OS_MACOS
namespace {
// Qt's default macOS behavior maps Command to Qt::ControlModifier and the
// physical Control key to Qt::MetaModifier (Qt::AA_MacDontSwapCtrlAndMeta is not
// set). The shared configuration keeps the Windows meaning of both modifiers,
// so swap them back before storing a trigger or building shortcut text.
Qt::KeyboardModifiers SharedModifiers(Qt::KeyboardModifiers modifiers) {
    auto result = modifiers & ~(Qt::ControlModifier | Qt::MetaModifier);
    if (modifiers & Qt::ControlModifier) result |= Qt::MetaModifier;
    if (modifiers & Qt::MetaModifier) result |= Qt::ControlModifier;
    return result;
}
}
#endif
InputRecorder::InputRecorder(EditorSession* session, QObject* parent) : QObject(parent), Session(session) {}
InputRecorder::~InputRecorder() { if (qApp) qApp->removeEventFilter(this); }
void InputRecorder::start(const QString& target) {
    cancel();
    Target = target; Profile = Session->profileId(); Item = Session->selectedId(); Active = true;
    qApp->installEventFilter(this); emit changed();
}
void InputRecorder::cancel() {
    if (!Active) return;
    Active = false; qApp->removeEventFilter(this); emit changed();
}
bool InputRecorder::eventFilter(QObject*, QEvent* event) {
    if (!Active) return false;
    if (event->type() == QEvent::ApplicationDeactivate) { cancel(); return false; }
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::MouseButtonPress) return false;
    if (Profile != Session->profileId() || (Target == "action" && Item != Session->selectedId())) { cancel(); return false; }
    Qt::KeyboardModifiers mods;
    uint32_t vk = 0; QString shortcut, button;
    if (event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) { cancel(); return true; }
        // Recording must never silently change the activation mode.
        if (Target == "trigger" && Session->profile()["triggerMode"].toInt() == 0) return true;
        if (key->isAutoRepeat() || key->key() == Qt::Key_Control || key->key() == Qt::Key_Shift ||
            key->key() == Qt::Key_Alt || key->key() == Qt::Key_Meta) return true;
        mods = key->modifiers(); vk = key->nativeVirtualKey();
#ifdef Q_OS_MACOS
        // Qt on macOS reports nativeVirtualKey() as the CGKeyCode; triggers and
        // shortcut text continue to store the shared Windows virtual key.
        if (vk) {
            const int mapped = WindowsVkFromMacKeyCode(int(vk));
            vk = mapped > 0 ? uint32_t(mapped) : 0;
        }
#endif
        if (!vk) { // QTest events have no native key code.
            const auto qt = key->key();
            if (qt >= Qt::Key_A && qt <= Qt::Key_Z) vk = uint32_t(qt);
            else if (qt >= Qt::Key_F1 && qt <= Qt::Key_F24) vk = 0x70 + qt - Qt::Key_F1;
            else if (qt >= Qt::Key_0 && qt <= Qt::Key_9) vk = uint32_t(qt);
        }
#ifdef Q_OS_MACOS
        // Command/Control meaning is swapped by Qt on macOS; translate before
        // the portable text is written so it uses the shared Win/Ctrl names.
        shortcut = QKeySequence(QKeyCombination(SharedModifiers(mods), key->key()))
            .toString(QKeySequence::PortableText).replace("Meta+", "Win+");
#else
        shortcut = QKeySequence(key->keyCombination()).toString(QKeySequence::PortableText).replace("Meta+", "Win+");
#endif
    } else {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (Target != "trigger" || mouse->button() == Qt::LeftButton) { cancel(); return false; }
        if (Session->profile()["triggerMode"].toInt() != 0) return true;
        mods = mouse->modifiers();
        switch (mouse->button()) {
        case Qt::RightButton: button = "Right"; break;
        case Qt::MiddleButton: button = "Middle"; break;
        case Qt::BackButton: button = "X1"; break;
        case Qt::ForwardButton: button = "X2"; break;
        default: return false;
        }
    }
#ifdef Q_OS_MACOS
    mods = SharedModifiers(mods);
#endif
    int modifiers = 0;
    if (mods & Qt::ControlModifier) modifiers |= 1;
    if (mods & Qt::ShiftModifier) modifiers |= 2;
    if (mods & Qt::AltModifier) modifiers |= 4;
    if (mods & Qt::MetaModifier) modifiers |= 8;
    if (Target == "action") Session->setItemField("target", shortcut);
    else {
        const auto profile = Session->profile();
        Session->setTrigger(profile["triggerMode"].toInt(), modifiers,
            button.isEmpty() ? profile["mouseButton"].toString() : button, int(vk));
    }
    cancel(); return true;
}
}
