import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root
    property int modifiers: 0
    signal edited(int value)
    spacing: 6
    // Qt maps the Command key to ControlModifier on macOS and the shared trigger
    // format keeps the Windows modifier names (the Win bit); only the editor
    // label and tooltip change so the UI matches the keyboard in use.
    readonly property bool isMacOS: Qt.platform.os === "osx"
    readonly property string primaryLabel: isMacOS ? qsTr("Cmd") : "Win"
    Repeater {
        model: [
            {label: "Ctrl", name: "Ctrl", bit: 1, hint: ""},
            {label: "Shift", name: "Shift", bit: 2, hint: ""},
            {label: "Alt", name: "Alt", bit: 4, hint: ""},
            {label: root.primaryLabel, name: "Win", bit: 8,
             hint: root.isMacOS ? qsTr("Command modifier") : ""}
        ]
        AppButton {
            required property var modelData
            objectName: "modifier" + modelData.name
            text: modelData.label
            hint: modelData.hint
            compact: true
            checkable: true
            checked: (root.modifiers & modelData.bit) !== 0
            Accessible.name: qsTr("%1 modifier").arg(modelData.label)
            onClicked: root.edited(root.modifiers ^ modelData.bit)
        }
    }
}
