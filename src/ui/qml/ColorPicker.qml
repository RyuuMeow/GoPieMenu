import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GoPieMenu 1.0
import "Theme.js" as T

AppDialog {
    id: root
    objectName: "colorPicker"
    title: "Choose color"
    width: Math.min(600, parent.width - 32)
    property color original: T.accent
    property bool hexValid: true
    property bool editingHex: false
    readonly property bool compact: parent.height < 600
    bodySpacing: compact ? 10 : 16
    function begin(color) {
        original = color
        wheel.color = color
        hexField.text = wheel.hex
        hexValid = true
        open()
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 20
        ColorWheel {
            id: wheel
            objectName: "colorWheel"
            Layout.preferredWidth: root.compact ? 216 : 248
            Layout.preferredHeight: Layout.preferredWidth
            onColorChanged: { if (!root.editingHex) hexField.text = hex; root.hexValid = true }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: root.compact ? 2 : 8
            ColorChannel { label: "H"; maximum: 359; value: wheel.hue * 360; onEdited: function(value) { wheel.hue = value / 360 } }
            ColorChannel { label: "S"; value: wheel.saturation * 100; onEdited: function(value) { wheel.saturation = value / 100 } }
            ColorChannel { label: "V"; value: wheel.value * 100; onEdited: function(value) { wheel.value = value / 100 } }
            ColorChannel { label: "A"; value: wheel.alpha * 100; onEdited: function(value) { wheel.alpha = value / 100 } }
            FieldLabel { text: "Hex · RRGGBB / RRGGBBAA"; topPadding: root.compact ? 0 : 8 }
            AppField {
                id: hexField
                objectName: "colorHexField"
                Layout.fillWidth: true
                maximumLength: 9
                placeholderText: "#RRGGBB"
                onTextEdited: {
                    root.editingHex = true
                    const valid = wheel.setHex(text)
                    root.editingHex = false
                    root.hexValid = valid
                }
                onEditingFinished: if (root.hexValid) text = wheel.hex
                Accessible.name: "Hex color"
            }
        }
    }
    Label { visible: !root.hexValid; text: "Enter 6 hex digits, or 8 including opacity."; color: T.danger; font.pixelSize: 12 }
    RowLayout {
        Layout.fillWidth: true
        ColorSwatch { value: root.original; Layout.preferredWidth: 64; Layout.preferredHeight: 32 }
        IconImage { name: "arrow-right.svg"; width: 16; height: 16; tint: T.muted }
        ColorSwatch { value: wheel.color; Layout.preferredWidth: 64; Layout.preferredHeight: 32 }
        Item { Layout.fillWidth: true }
        AppButton { text: "Reset"; kind: "danger"; compact: root.compact; onClicked: { wheel.color = root.original; hexField.text = wheel.hex } }
    }
    FieldLabel { text: "Recent colors"; topPadding: root.compact ? 0 : 8 }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        Repeater {
            model: appController.recentColors
            AppButton {
                required property string modelData
                required property int index
                objectName: "recentColor" + index
                implicitWidth: 36; implicitHeight: 36
                hint: "Use " + modelData
                ColorSwatch { anchors.fill: parent; anchors.margins: 3; value: parent.modelData }
                onClicked: { wheel.color = modelData; hexField.text = wheel.hex }
            }
        }
        Label { visible: appController.recentColors.length === 0; text: "Colors you use will appear here."; color: T.muted; font.pixelSize: 13 }
    }
    footer: RowLayout {
        Item { Layout.fillWidth: true }
        AppButton { objectName: "cancelColorButton"; text: "Cancel"; onClicked: root.close() }
        AppButton { objectName: "useColorButton"; text: "Use color"; kind: "primary"; enabled: root.hexValid; onClicked: { appController.acceptColor(wheel.color); root.close() } }
    }
}
