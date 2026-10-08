import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

ColumnLayout {
    id: root
    property var item: editor.selectedItem
    property bool advanced: false
    signal chooseIcon()
    spacing: 8
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        AppButton {
            objectName: "chooseIconButton"
            kind: "secondary"
            iconName: root.item.icon || "media-image.svg"
            hint: "Choose icon"
            implicitHeight: 46; implicitWidth: 46
            onClicked: root.chooseIcon()
        }
        AppField {
            objectName: "itemNameField"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            font.pixelSize: 17
            font.weight: Font.DemiBold
            placeholderText: "Action name"
            text: root.item.name || ""
            onTextEdited: editor.setItemField("name", text)
            Accessible.name: "Action name"
        }
    }
    FieldLabel { text: "Action" }
    AppCombo {
        objectName: "actionTypeCombo"
        Layout.fillWidth: true
        textRole: "label"; valueRole: "value"
        model: [
            {label: "Open application", value: 1},
            {label: "Keyboard shortcut", value: 3},
            {label: "Open file or folder", value: 4},
            {label: "Open website", value: 5},
            {label: "Run command", value: 2},
            {label: "Submenu", value: 6}
        ]
        selectionValue: root.item.action
        onActivated: editor.setItemField("action", currentValue)
        enabled: !(root.item.action === 6 && root.item.childCount > 0)
        Accessible.name: "Action type"
    }
    FieldLabel {
        visible: root.item.action !== 6
        text: root.item.action === 3 ? "Shortcut" : root.item.action === 5 ? "Website" : root.item.action === 2 ? "Command" : "Target"
    }
    AppButton {
        objectName: "actionRecorderButton"
        visible: root.item.action === 3
        Layout.fillWidth: true
        kind: "secondary"
        iconName: "key-command.svg"
        text: recorder.active && recorder.target === "action" ? "Press a shortcut…" : root.item.target || "Record shortcut"
        onClicked: recorder.active ? recorder.cancel() : recorder.start("action")
    }
    AppField {
        objectName: "targetField"
        visible: root.item.action !== 3 && root.item.action !== 6
        Layout.fillWidth: true
        text: root.item.target || ""
        placeholderText: root.item.action === 5 ? "https://example.com" : root.item.action === 2 ? "Enter a command" : "Choose a target or paste a path"
        onTextEdited: editor.setItemField("target", text)
        Accessible.name: "Action target"
    }
    RowLayout {
        visible: root.item.action === 1 || root.item.action === 4
        Layout.fillWidth: true
        AppButton {
            text: root.item.action === 1 ? "Choose application" : "Choose file"
            iconName: root.item.action === 1 ? "app-window.svg" : "page.svg"
            kind: "secondary"
            onClicked: appController.chooseTarget(false)
        }
        AppButton { visible: root.item.action === 4; iconName: "folder.svg"; hint: "Choose folder"; kind: "secondary"; onClicked: appController.chooseTarget(true) }
    }
    AppButton {
        objectName: "enterSubmenuButton"
        visible: root.item.action === 6
        Layout.fillWidth: true
        kind: "secondary"
        text: "Edit " + (root.item.childCount || 0) + " actions"
        iconName: "nav-arrow-right.svg"
        onClicked: editor.enterFolder(root.item.id)
    }
    AppButton { objectName: "itemAdvancedButton"; text: "Advanced"; kind: "text"; iconName: root.advanced ? "nav-arrow-down.svg" : "nav-arrow-right.svg"; onClicked: root.advanced = !root.advanced }
    ColumnLayout {
        visible: root.advanced
        Layout.fillWidth: true
        spacing: 8
        FieldLabel { visible: root.item.action === 1 || root.item.action === 2; text: "Arguments" }
        AppField {
            visible: root.item.action === 1 || root.item.action === 2
            Layout.fillWidth: true
            text: root.item.arguments || ""
            placeholderText: "Optional arguments"
            onTextEdited: editor.setItemField("arguments", text)
        }
        FieldLabel { text: "Color" }
        RowLayout {
            AppButton { text: root.item.color ? "Change color" : "Custom color"; kind: "text"; onClicked: appController.chooseColor("color", true) }
            Rectangle { visible: !!root.item.color; width: 22; height: 22; radius: 5; color: root.item.color || "transparent"; border.color: T.line }
            AppButton { objectName: "resetItemColorButton"; visible: !!root.item.color; text: "Reset"; kind: "danger"; onClicked: editor.setItemField("color", "") }
        }
        FieldLabel { visible: root.item.action !== 6; text: "Location" }
        AppCombo {
            visible: root.item.action !== 6
            Layout.fillWidth: true
            model: editor.destinations
            textRole: "name"; valueRole: "id"
            selectionValue: editor.folderId
            onActivated: editor.moveItem(root.item.id, currentValue, 99999)
        }
    }
    Item { implicitHeight: 16 }
    Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: T.line }
    RowLayout {
        Layout.fillWidth: true
        AppButton { text: "Duplicate"; iconName: "copy.svg"; onClicked: editor.duplicateItem(root.item.id) }
        Item { Layout.fillWidth: true }
        AppButton { iconName: "trash.svg"; hint: "Delete action"; kind: "danger"; onClicked: editor.removeItem(root.item.id) }
    }
}
