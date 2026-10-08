import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T
Button {
    id: root
    property string iconName: ""
    property string kind: "quiet"
    property string hint: ""
    property bool compact: false
    implicitHeight: compact ? 34 : 40
    implicitWidth: Math.max(compact ? 34 : 40, contentItem.implicitWidth + (text.length ? 24 : 16))
    padding: 8
    horizontalPadding: text.length ? 12 : 8
    hoverEnabled: true
    font.pixelSize: 14
    focusPolicy: Qt.StrongFocus
    Accessible.name: text.length ? text : hint
    ToolTip.visible: hovered && hint.length > 0
    ToolTip.text: hint
    ToolTip.delay: 650
    contentItem: Row {
        spacing: root.iconName.length && root.text.length ? 8 : 0
        opacity: root.enabled ? 1 : .35
        IconImage {
            visible: root.iconName.length > 0
            width: visible ? 19 : 0; height: 19
            anchors.verticalCenter: parent.verticalCenter
            name: root.iconName
            tint: root.kind === "primary" ? "#ffffff" : root.kind === "danger" ? T.danger : T.ink
        }
        Text {
            text: root.text
            font: root.font
            color: root.kind === "primary" ? "#ffffff" : root.kind === "danger" ? T.danger : T.ink
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    background: Rectangle {
        radius: 8
        color: root.kind === "primary" ? (root.down ? "#3a69ae" : root.hovered ? "#4375bd" : T.accent)
             : root.down ? "#e3ebf7" : root.hovered || root.checked ? T.tint : root.kind === "secondary" ? T.surface : "transparent"
        border.width: root.activeFocus || root.kind === "secondary" ? 1 : 0
        border.color: root.activeFocus ? T.accent : T.line
        opacity: root.enabled ? 1 : .5
    }
}
