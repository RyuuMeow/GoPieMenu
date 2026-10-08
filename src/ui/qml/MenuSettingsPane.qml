import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

ColumnLayout {
    spacing: 10
    FieldLabel { text: "Menu name" }
    AppField { Layout.fillWidth: true; text: editor.profile.name || ""; onTextEdited: editor.setProfileField("name", text); Accessible.name: "Menu name" }
    Switch { text: "Enabled"; font.pixelSize: 14; checked: editor.profile.enabled || false; onClicked: editor.setProfileField("enabled", checked) }
    FieldLabel { text: "Open with" }
    AppButton {
        objectName: "triggerRecorderButton"
        Layout.fillWidth: true; kind: "secondary"; iconName: "key-command.svg"
        text: recorder.active && recorder.target === "trigger" ? "Press keys or a mouse button…" : editor.profile.triggerSummary || "Record trigger"
        onClicked: recorder.active ? recorder.cancel() : recorder.start("trigger")
    }
    AppCombo {
        Layout.fillWidth: true
        model: ["Hold mouse button", "Hold keyboard shortcut", "Press to toggle"]
        currentIndex: editor.profile.triggerMode || 0
        onActivated: editor.setProfileField("triggerMode", currentIndex)
        Accessible.name: "Activation mode"
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
