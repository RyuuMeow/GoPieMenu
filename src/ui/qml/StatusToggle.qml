import QtQuick
import QtQuick.Controls
import "Theme.js" as T

Switch {
    id: root
    property bool canEnable: true
    enabled: checked || canEnable
    implicitWidth: 126; implicitHeight: 34
    padding: 0
    spacing: 10
    hoverEnabled: true
    text: checked ? qsTr("Enabled") : qsTr("Disabled")
    Accessible.name: qsTr("Menu enabled")
    indicator: Rectangle {
        width: 42; height: 24
        y: (root.height - height)/2
        radius: 12
        color: root.checked ? T.success : T.danger
        opacity: root.enabled ? 1 : .5
        border.color: root.visualFocus ? T.accent : "transparent"
        border.width: 2
        Rectangle { x: root.checked ? 22 : 4; y: 4; width: 16; height: 16; radius: 8; color: "#ffffff" }
    }
    contentItem: Text {
        leftPadding: 52
        text: root.text
        color: root.checked ? T.success : T.danger
        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
    }
    AppToolTip { visible: root.hovered; text: root.checked ? qsTr("Disable this menu") : root.canEnable ? qsTr("Enable this menu") : qsTr("Add an action before enabling this menu") }
}
