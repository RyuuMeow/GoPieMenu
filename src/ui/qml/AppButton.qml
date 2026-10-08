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
    property bool tooltipEnabled: true
    property int maximumTextWidth: 240
    property int iconSize: T.iconSize
    readonly property bool textOnly: kind === "text" || kind === "danger"
    readonly property bool filled: kind === "primary"
    readonly property color foreground: filled ? "#ffffff" : kind === "danger" ? T.danger : textOnly && (hovered || down) ? T.accent : T.ink
    implicitHeight: compact ? T.compactHeight : T.controlHeight
    implicitWidth: Math.max(compact ? T.compactHeight : T.controlHeight, contents.implicitWidth + (text.length ? 24 : 16))
    padding: 8
    horizontalPadding: text.length ? 12 : 8
    hoverEnabled: true
    font.pixelSize: 14
    focusPolicy: Qt.StrongFocus
    Accessible.name: text.length ? text : hint
    AppToolTip {
        objectName: root.objectName + "Tip"
        visible: root.enabled && root.hovered && root.tooltipEnabled && text.length > 0
        text: root.hint
    }
    contentItem: Item {
        opacity: root.enabled ? 1 : .35
        Row {
            id: contents
            anchors.verticalCenter: parent.verticalCenter
            x: root.text.length ? 0 : (parent.width - width) / 2
            spacing: root.iconName.length && root.text.length ? 8 : 0
            IconImage {
                objectName: "buttonIcon"
                visible: root.iconName.length > 0
                width: visible ? root.iconSize : 0; height: root.iconSize
                anchors.verticalCenter: parent.verticalCenter
                name: root.iconName
                tint: root.foreground
            }
            Text {
                text: root.text
                width: Math.min(implicitWidth, root.maximumTextWidth)
                elide: Text.ElideRight
                font.family: root.font.family
                font.pixelSize: root.font.pixelSize
                font.weight: root.font.weight
                color: root.foreground
                font.underline: root.textOnly && root.visualFocus
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
    background: Rectangle {
        radius: T.radius
        color: root.kind === "primary" ? (root.down ? "#3a69ae" : root.hovered ? "#4375bd" : T.accent)
             : root.textOnly ? "transparent"
             : root.down ? T.pressed : root.hovered || root.checked ? T.hover : root.kind === "secondary" ? T.surface : "transparent"
        border.width: !root.textOnly && (root.visualFocus || root.kind === "secondary") ? 1 : 0
        border.color: root.visualFocus ? T.accent : T.line
        opacity: root.enabled ? 1 : .5
    }
}
