import QtQuick
import QtQuick.Controls

AppMenu {
    id: root
    objectName: "actionContextMenu"
    property string itemId: ""
    property string profileId: ""
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    function showFor(id, position) {
        itemId = id
        profileId = editor.profileId
        popup(position.x, position.y)
    }
    AppMenuItem {
        objectName: "contextDuplicateAction"
        text: "Duplicate"
        onTriggered: if (root.profileId === editor.profileId) editor.duplicateItem(root.itemId)
    }
    AppMenuItem {
        objectName: "contextDeleteAction"
        text: "Delete action"
        destructive: true
        onTriggered: if (root.profileId === editor.profileId) editor.removeItem(root.itemId)
    }
}
