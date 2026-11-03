import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 8
    AppCheckBox {
        objectName: "overlayEnabled"
        text: qsTr("Show a hint when an action runs")
        checked: editor.actionOverlay.enabled
        onClicked: editor.setOverlayField("enabled", checked)
    }
    ColumnLayout {
        visible: editor.actionOverlay.enabled
        Layout.fillWidth: true
        spacing: 8
        FieldLabel { text: qsTr("Screen position") }
        GridLayout {
            Layout.fillWidth: true
            columns: 3
            Repeater {
                model: [qsTr("Top left"), qsTr("Top center"), qsTr("Top right"),
                        qsTr("Bottom left"), qsTr("Bottom center"), qsTr("Bottom right")]
                AppButton {
                    required property int index
                    required property string modelData
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: modelData
                    kind: editor.actionOverlay.position === index ? "primary" : "secondary"
                    onClicked: editor.setOverlayField("position", index)
                }
            }
        }
        FieldLabel { text: qsTr("Hint size") }
        AppCombo {
            Layout.fillWidth: true
            model: [qsTr("Small"), qsTr("Medium"), qsTr("Large")]
            currentIndex: editor.actionOverlay.size
            onActivated: editor.setOverlayField("size", currentIndex)
            Accessible.name: qsTr("Hint size")
        }
        NumberSetting {
            Layout.fillWidth: true
            label: qsTr("Background opacity"); field: "opacity"; suffix: "%"
            value: editor.actionOverlay.opacity
            writeValue: function(v) { editor.setOverlayField(field, v) }
        }
        NumberSetting {
            Layout.fillWidth: true
            label: qsTr("Edge margin"); field: "margin"; maximum: 160; suffix: " px"
            value: editor.actionOverlay.margin
            writeValue: function(v) { editor.setOverlayField(field, v) }
        }
        NumberSetting {
            Layout.fillWidth: true
            label: qsTr("Hold time"); field: "durationMs"; minimum: 250; maximum: 5000; step: 250; suffix: " ms"
            value: editor.actionOverlay.durationMs
            writeValue: function(v) { editor.setOverlayField(field, v) }
        }
        AppButton {
            objectName: "overlayPreviewButton"
            text: qsTr("Preview hint"); iconName: "play.svg"; kind: "secondary"
            onClicked: appController.previewActionOverlay()
        }
    }
}
