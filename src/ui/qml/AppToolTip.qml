import QtQuick
import QtQuick.Controls
import "Theme.js" as T

ToolTip {
    id: root
    delay: 650
    padding: 8
    implicitWidth: Math.min(300, contentItem.implicitWidth + 16)
    contentItem: Text {
        text: root.text
        color: T.ink
        font.pixelSize: 12
        wrapMode: Text.Wrap
    }
    background: Rectangle { color: T.surface; border.color: T.line; radius: 6 }
}
