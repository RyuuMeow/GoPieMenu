import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

ColumnLayout {
    id: root
    property bool advanced: false
    property var style: editor.style
    spacing: 14
    Label {
        Layout.fillWidth: true
        text: "Appearance applies only to this menu."
        color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
    }
    FieldLabel { text: "Style" }
    Flow {
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: ["Frost", "Slate", "Ocean", "Obsidian"]
            AppButton { required property string modelData; text: modelData; kind: "secondary"; onClicked: editor.setStylePreset(modelData) }
        }
    }
    NumberSetting { Layout.fillWidth: true; label: "Size"; field: "outerRadius"; minimum: 80; maximum: 400; step: 5; value: root.style.outerRadius || 150 }
    AppButton { text: "Advanced"; kind: "text"; iconName: root.advanced ? "nav-arrow-down.svg" : "nav-arrow-right.svg"; onClicked: root.advanced = !root.advanced }
    ColumnLayout {
        Layout.fillWidth: true
        visible: root.advanced
        spacing: 16
        NumberSetting { Layout.fillWidth: true; label: "Center radius"; field: "innerRadius"; minimum: 10; maximum: Math.min(150, (root.style.outerRadius || 150) - 9); value: root.style.innerRadius || 45 }
        NumberSetting { Layout.fillWidth: true; label: "Icon size"; field: "iconSize"; minimum: 12; maximum: 96; value: root.style.iconSize || 28 }
        NumberSetting { Layout.fillWidth: true; label: "Text size"; field: "fontSize"; minimum: 8; maximum: 32; value: root.style.fontSize || 11 }
        NumberSetting { Layout.fillWidth: true; label: "Slice gap"; field: "gapAngle"; minimum: 0; maximum: 20; step: .5; value: root.style.gapAngle || 0; suffix: "°" }
        NumberSetting { Layout.fillWidth: true; label: "Opacity"; field: "opacity"; minimum: .1; maximum: 1; step: .05; value: root.style.opacity || .95 }
        NumberSetting { Layout.fillWidth: true; label: "Animation"; field: "animationDuration"; minimum: 0; maximum: 500; step: 10; value: root.style.animationDuration || 0; suffix: " ms" }
        NumberSetting { Layout.fillWidth: true; label: "Text outline"; field: "textOutlineThickness"; minimum: 0; maximum: 10; step: .5; value: root.style.textOutlineThickness || 0 }
        NumberSetting { Layout.fillWidth: true; label: "Border width"; field: "borderWidth"; minimum: 0; maximum: 5; step: .5; value: root.style.borderWidth || 0 }
        NumberSetting { Layout.fillWidth: true; label: "Hover size"; field: "hoverScale"; minimum: 1; maximum: 1.25; step: .01; value: root.style.hoverScale || 1 }
        AppCheckBox { text: "Automatic text contrast"; checked: root.style.autoContrast || false; onClicked: editor.setStyleField("autoContrast", checked) }
        Repeater {
            model: [
                {name: "Slices", key: "sectorColor"}, {name: "Highlight", key: "hoverColor"},
                {name: "Text", key: "textColor"}, {name: "Center", key: "centerColor"},
                {name: "Border", key: "borderColor"}, {name: "Submenu", key: "backgroundColor"}
            ]
            RowLayout {
                required property var modelData
                Layout.fillWidth: true
                Label { text: modelData.name; font.pixelSize: 14; color: T.ink; Layout.fillWidth: true }
                AppButton {
                    kind: "secondary"; implicitWidth: 64
                    hint: "Change " + modelData.name.toLowerCase() + " color"
                    onClicked: appController.chooseColor(modelData.key)
                    Rectangle { width: 28; height: 18; anchors.centerIn: parent; color: root.style[modelData.key] || "transparent"; radius: 4; border.color: T.line }
                }
            }
        }
    }
}
