import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

ColumnLayout {
    spacing: 10
    FieldLabel { text: qsTr("Menu name") }
    AppField { Layout.fillWidth: true; text: editor.profile.name || ""; onTextEdited: editor.setProfileField("name", text); Accessible.name: qsTr("Menu name") }
    StatusToggle {
        objectName: "settingsEnabledToggle"
        checked: editor.profile.enabled || false
        canEnable: (editor.profile.itemCount || 0) > 0
        onToggled: editor.setProfileField("enabled", checked)
    }
    FieldLabel { text: qsTr("Open with") }
    AppCombo {
        objectName: "activationModeCombo"
        Layout.fillWidth: true
        model: [qsTr("Hold mouse button"), qsTr("Hold keyboard shortcut"), qsTr("Press to toggle")]
        currentIndex: editor.profile.triggerMode || 0
        onActivated: { recorder.cancel(); editor.setProfileField("triggerMode", currentIndex) }
        Accessible.name: qsTr("Activation mode")
    }
    FieldLabel { text: qsTr("Modifiers") }
    ModifierButtons { modifiers: editor.profile.modifiers || 0; onEdited: function(value) { editor.setProfileField("modifiers", value) } }
    FieldLabel { text: editor.profile.triggerMode === 0 ? qsTr("Mouse button") : qsTr("Keyboard shortcut") }
    AppCombo {
        objectName: "mouseButtonCombo"
        visible: editor.profile.triggerMode === 0
        Layout.fillWidth: true
        textRole: "label"; valueRole: "value"
        model: [
            {label: qsTr("Left button"), value: "Left"}, {label: qsTr("Right button"), value: "Right"},
            {label: qsTr("Middle button"), value: "Middle"}, {label: qsTr("Side button 1 (X1)"), value: "X1"},
            {label: qsTr("Side button 2 (X2)"), value: "X2"}
        ]
        selectionValue: editor.profile.mouseButton
        onActivated: editor.setProfileField("mouseButton", currentValue)
        Accessible.name: qsTr("Trigger mouse button")
    }
    AppButton {
        objectName: "triggerRecorderButton"
        visible: editor.profile.triggerMode !== 0
        Layout.fillWidth: true; kind: "secondary"; iconName: "key-command.svg"
        text: recorder.active && recorder.target === "trigger" ? qsTr("Press a keyboard shortcut…") : editor.profile.vkCode ? editor.profile.triggerSummary : qsTr("Record shortcut")
        onClicked: recorder.active ? recorder.cancel() : recorder.start("trigger")
    }
    Label {
        Layout.fillWidth: true
        text: editor.profile.triggerMode === 2 ? qsTr("Press the shortcut, point to an action, then click to run it.") : qsTr("Hold the trigger, point to an action, then release to run it.")
        wrapMode: Text.WordWrap; color: T.muted; font.pixelSize: 13
    }
    FieldLabel { text: qsTr("Available in") }
    AppField {
        Layout.fillWidth: true
        text: editor.profile.appFilter || ""
        placeholderText: qsTr("All applications")
        onEditingFinished: editor.setProfileField("appFilter", text)
        Accessible.name: qsTr("Application filter")
    }
    RowLayout {
        AppButton { text: qsTr("Running apps"); iconName: "app-window.svg"; kind: "secondary"; onClicked: appController.pickRunningApplication() }
        AppButton { iconName: "folder.svg"; hint: qsTr("Choose application"); kind: "secondary"; onClicked: appController.chooseApplicationFilter() }
    }
    Label { Layout.fillWidth: true; text: qsTr("Leave empty for every app. Separate application names with commas."); color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap }
}
