import QtQuick
import QtQuick.Controls
import "Theme.js" as T
TextField {
    id: root
    implicitHeight: 42
    implicitWidth: 200
    font.pixelSize: 14
    color: T.ink
    placeholderTextColor: "#99a3b3"
    selectByMouse: true
    selectionColor: "#d8e6fa"
    selectedTextColor: T.ink
    padding: 11
    background: Rectangle {
        color: root.enabled ? T.surface : "#f4f6f9"
        radius: 8
        border.width: 1
        border.color: root.activeFocus ? T.accent : T.line
    }
}
