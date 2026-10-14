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
            hint: qsTr("Choose icon")
            implicitHeight: 46; implicitWidth: 46
            onClicked: root.chooseIcon()
        }
        AppField {
            objectName: "itemNameField"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            font.pixelSize: 17
            font.weight: Font.DemiBold
            placeholderText: qsTr("Action name")
            text: root.item.name || ""
            onTextEdited: editor.setItemField("name", text)
            Accessible.name: qsTr("Action name")
        }
    }
    FieldLabel { text: qsTr("Action") }
    AppCombo {
        objectName: "actionTypeCombo"
        Layout.fillWidth: true
        textRole: "label"; valueRole: "value"
        model: [
            {label: qsTr("Open application"), value: 1},
            {label: qsTr("Keyboard shortcut"), value: 3},
            {label: qsTr("Open file or folder"), value: 4},
            {label: qsTr("Open website"), value: 5},
            {label: qsTr("Run command"), value: 2},
            {label: qsTr("Submenu"), value: 6}
        ]
        selectionValue: root.item.action
        onActivated: editor.setItemField("action", currentValue)
        enabled: !(root.item.action === 6 && root.item.childCount > 0)
        Accessible.name: qsTr("Action type")
    }
    FieldLabel {
        visible: root.item.action !== 6
        text: root.item.action === 3 ? qsTr("Shortcut") : root.item.action === 5 ? qsTr("Website") : root.item.action === 2 ? qsTr("Command") : qsTr("Target")
    }
    AppButton {
        objectName: "actionRecorderButton"
        visible: root.item.action === 3
        Layout.fillWidth: true
        kind: "secondary"
        iconName: "key-command.svg"
        text: recorder.active && recorder.target === "action" ? qsTr("Press a shortcut…") : root.item.target || qsTr("Record shortcut")
        onClicked: recorder.active ? recorder.cancel() : recorder.start("action")
    }
    AppField {
        objectName: "targetField"
        visible: root.item.action !== 3 && root.item.action !== 6
        Layout.fillWidth: true
        text: root.item.target || ""
        placeholderText: root.item.action === 5 ? "https://example.com" : root.item.action === 2 ? qsTr("Enter a command") : qsTr("Choose a target or paste a path")
        onTextEdited: editor.setItemField("target", text)
        Accessible.name: qsTr("Action target")
    }
    RowLayout {
        visible: root.item.action === 1 || root.item.action === 4
        Layout.fillWidth: true
        AppButton {
            text: root.item.action === 1 ? qsTr("Choose application") : qsTr("Choose file")
            iconName: root.item.action === 1 ? "app-window.svg" : "page.svg"
            kind: "secondary"
            onClicked: appController.chooseTarget(false)
        }
        AppButton { visible: root.item.action === 4; iconName: "folder.svg"; hint: qsTr("Choose folder"); kind: "secondary"; onClicked: appController.chooseTarget(true) }
    }
    AppButton {
        objectName: "enterSubmenuButton"
        visible: root.item.action === 6
        Layout.fillWidth: true
        kind: "secondary"
        text: qsTr("Edit %1 actions").arg(root.item.childCount || 0)
        iconName: "nav-arrow-right.svg"
        onClicked: editor.enterFolder(root.item.id)
    }
    AppButton { objectName: "itemAdvancedButton"; text: qsTr("Advanced"); kind: "text"; iconName: root.advanced ? "nav-arrow-down.svg" : "nav-arrow-right.svg"; onClicked: root.advanced = !root.advanced }
    ColumnLayout {
        visible: root.advanced
        Layout.fillWidth: true
        spacing: 8
        FieldLabel { visible: root.item.action === 1 || root.item.action === 2; text: qsTr("Arguments") }
        AppField {
            visible: root.item.action === 1 || root.item.action === 2
            Layout.fillWidth: true
            text: root.item.arguments || ""
            placeholderText: qsTr("Optional arguments")
            onTextEdited: editor.setItemField("arguments", text)
        }
        FieldLabel { text: qsTr("Color") }
        RowLayout {
            AppButton { text: root.item.color ? qsTr("Change color") : qsTr("Custom color"); kind: "text"; onClicked: appController.chooseColor("color", true) }
            Rectangle { visible: !!root.item.color; width: 22; height: 22; radius: 5; color: root.item.color || "transparent"; border.color: T.line }
            AppButton { objectName: "resetItemColorButton"; visible: !!root.item.color; text: qsTr("Reset"); kind: "danger"; onClicked: editor.setItemField("color", "") }
        }
        FieldLabel { visible: root.item.action !== 6; text: qsTr("Location") }
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
        AppButton { text: qsTr("Duplicate"); iconName: "copy.svg"; onClicked: editor.duplicateItem(root.item.id) }
        Item { Layout.fillWidth: true }
        AppButton { iconName: "trash.svg"; hint: qsTr("Delete action"); kind: "danger"; onClicked: editor.removeItem(root.item.id) }
    }
}
