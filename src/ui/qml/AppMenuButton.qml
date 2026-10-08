import QtQuick

AppButton {
    id: root
    required property var menu
    checked: menu.visible
    tooltipEnabled: !menu.visible
    onClicked: menu.visible ? menu.close() : menu.open()
}
