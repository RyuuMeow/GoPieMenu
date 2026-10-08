import QtQuick
import QtQuick.Controls
import "Theme.js" as T

CheckBox {
    id: root
    implicitHeight: 32
    font.pixelSize: 14
    padding: 0; spacing: 10
    indicator: Rectangle {
        width: 18; height: 18; radius: 4
        y: (root.height - height)/2
        color: root.checked ? T.accent : T.surface
        border.color: root.visualFocus || root.hovered ? T.accent : T.line
        IconImage { anchors.fill: parent; anchors.margins: 2; name: root.checked ? "check.svg" : ""; tint: "#ffffff" }
    }
    contentItem: Text { text: root.text; font: root.font; color: T.ink; leftPadding: 28; verticalAlignment: Text.AlignVCenter }
}
