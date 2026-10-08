import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

ColumnLayout {
    spacing: 10
    FieldLabel { text: "Menu name" }
    AppField { Layout.fillWidth: true; text: editor.profile.name || ""; onTextEdited: editor.setProfileField("name", text); Accessible.name: "Menu name" }
    StatusToggle {
        objectName: "settingsEnabledToggle"
        checked: editor.profile.enabled || false
        canEnable: (editor.profile.itemCount || 0) > 0
        onToggled: editor.setProfileField("enabled", checked)
    }
    FieldLabel { text: "Open with" }
    AppCombo {
        objectName: "activationModeCombo"
        Layout.fillWidth: true
        model: ["Hold mouse button", "Hold keyboard shortcut", "Press to toggle"]
        currentIndex: editor.profile.triggerMode || 0
        onActivated: { recorder.cancel(); editor.setProfileField("triggerMode", currentIndex) }
        Accessible.name: "Activation mode"
    }
    FieldLabel { text: "Modifiers" }
    ModifierButtons { modifiers: editor.profile.modifiers || 0; onEdited: function(value) { editor.setProfileField("modifiers", value) } }
    FieldLabel { text: editor.profile.triggerMode === 0 ? "Mouse button" : "Keyboard shortcut" }
    AppCombo {
        objectName: "mouseButtonCombo"
        visible: editor.profile.triggerMode === 0
        Layout.fillWidth: true
        textRole: "label"; valueRole: "value"
        model: [
            {label: "Left button", value: "Left"}, {label: "Right button", value: "Right"},
            {label: "Middle button", value: "Middle"}, {label: "Side button 1 (X1)", value: "X1"},
            {label: "Side button 2 (X2)", value: "X2"}
        ]
        selectionValue: editor.profile.mouseButton
        onActivated: editor.setProfileField("mouseButton", currentValue)
        Accessible.name: "Trigger mouse button"
    }
    AppButton {
        objectName: "triggerRecorderButton"
        visible: editor.profile.triggerMode !== 0
        Layout.fillWidth: true; kind: "secondary"; iconName: "key-command.svg"
        text: recorder.active && recorder.target === "trigger" ? "Press a keyboard shortcut…" : editor.profile.vkCode ? editor.profile.triggerSummary : "Record shortcut"
        onClicked: recorder.active ? recorder.cancel() : recorder.start("trigger")
    }
    Label {
        Layout.fillWidth: true
        text: editor.profile.triggerMode === 2 ? "Press the shortcut, point to an action, then click to run it." : "Hold the trigger, point to an action, then release to run it."
        wrapMode: Text.WordWrap; color: T.muted; font.pixelSize: 13
    }
    FieldLabel { text: "Available in" }
    AppField {
        Layout.fillWidth: true
        text: editor.profile.appFilter || ""
        placeholderText: "All applications"
        onEditingFinished: editor.setProfileField("appFilter", text)
        Accessible.name: "Application filter"
    }
    RowLayout {
        AppButton { text: "Running apps"; iconName: "app-window.svg"; kind: "secondary"; onClicked: appController.pickRunningApplication() }
        AppButton { iconName: "folder.svg"; hint: "Choose application"; kind: "secondary"; onClicked: appController.chooseApplicationFilter() }
    }
    Label { Layout.fillWidth: true; text: "Leave empty for every app. Separate application names with commas."; color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap }
}
