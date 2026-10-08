#include "InputRecorder.h"
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QKeySequence>

namespace gpm {
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
        if (!vk) { // QTest events have no native key code.
            const auto qt = key->key();
            if (qt >= Qt::Key_A && qt <= Qt::Key_Z) vk = uint32_t(qt);
            else if (qt >= Qt::Key_F1 && qt <= Qt::Key_F24) vk = 0x70 + qt - Qt::Key_F1;
            else if (qt >= Qt::Key_0 && qt <= Qt::Key_9) vk = uint32_t(qt);
        }
        shortcut = QKeySequence(key->keyCombination()).toString(QKeySequence::PortableText).replace("Meta+", "Win+");
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
