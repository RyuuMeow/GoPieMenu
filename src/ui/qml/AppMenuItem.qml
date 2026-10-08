import QtQuick
import QtQuick.Controls
import "Theme.js" as T

MenuItem {
    id: root
    property bool destructive: false
    implicitHeight: T.controlHeight
    leftPadding: 12; rightPadding: 12
    contentItem: Text {
        text: root.text
        color: root.destructive ? T.danger : T.ink
        font.pixelSize: 14
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        opacity: root.enabled ? 1 : .4
    }
    background: Rectangle {
        radius: 5
        color: root.down ? T.pressed : root.highlighted ? T.hover : "transparent"
    }
}
