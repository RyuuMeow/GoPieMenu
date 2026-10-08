import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root
    property int modifiers: 0
    signal edited(int value)
    spacing: 6
    Repeater {
        model: [{label: "Ctrl", bit: 1}, {label: "Shift", bit: 2}, {label: "Alt", bit: 4}, {label: "Win", bit: 8}]
        AppButton {
            required property var modelData
            objectName: "modifier" + modelData.label
            text: modelData.label
            compact: true
            checkable: true
            checked: (root.modifiers & modelData.bit) !== 0
            Accessible.name: modelData.label + " modifier"
            onClicked: root.edited(root.modifiers ^ modelData.bit)
        }
    }
}
