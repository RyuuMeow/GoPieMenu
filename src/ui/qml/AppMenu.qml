import QtQuick
import QtQuick.Controls
import "Theme.js" as T

Menu {
    id: root
    width: 224
    padding: 6
    y: parent ? parent.height + 6 : 0
    modal: false
    focus: true
    popupType: Popup.Item
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    delegate: AppMenuItem {}
    background: Rectangle { color: T.surface; border.color: T.line; radius: T.radius }
}
