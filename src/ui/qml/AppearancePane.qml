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
        text: qsTr("Appearance applies only to this menu.")
        color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
    }
    FieldLabel { text: qsTr("Style") }
    Flow {
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: [
                {label: qsTr("Frost"), value: "Frost"}, {label: qsTr("Slate"), value: "Slate"},
                {label: qsTr("Ocean"), value: "Ocean"}, {label: qsTr("Obsidian"), value: "Obsidian"}
            ]
            AppButton { required property var modelData; text: modelData.label; kind: "secondary"; onClicked: editor.setStylePreset(modelData.value) }
        }
    }
    NumberSetting { Layout.fillWidth: true; label: qsTr("Size"); field: "outerRadius"; minimum: 80; maximum: 400; step: 5; value: root.style.outerRadius || 150 }
    AppButton { text: qsTr("Advanced"); kind: "text"; iconName: root.advanced ? "nav-arrow-down.svg" : "nav-arrow-right.svg"; onClicked: root.advanced = !root.advanced }
    ColumnLayout {
        Layout.fillWidth: true
        visible: root.advanced
        spacing: 16
        NumberSetting { Layout.fillWidth: true; label: qsTr("Center radius"); field: "innerRadius"; minimum: 10; maximum: Math.min(150, (root.style.outerRadius || 150) - 9); value: root.style.innerRadius || 45 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Icon size"); field: "iconSize"; minimum: 12; maximum: 96; value: root.style.iconSize || 28 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Text size"); field: "fontSize"; minimum: 8; maximum: 32; value: root.style.fontSize || 11 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Slice gap"); field: "gapAngle"; minimum: 0; maximum: 20; step: .5; value: root.style.gapAngle || 0; suffix: "°" }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Opacity"); field: "opacity"; minimum: .1; maximum: 1; step: .05; value: root.style.opacity || .95 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Animation"); field: "animationDuration"; minimum: 0; maximum: 500; step: 10; value: root.style.animationDuration || 0; suffix: " ms" }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Text outline"); field: "textOutlineThickness"; minimum: 0; maximum: 10; step: .5; value: root.style.textOutlineThickness || 0 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Border width"); field: "borderWidth"; minimum: 0; maximum: 5; step: .5; value: root.style.borderWidth || 0 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Hover size"); field: "hoverScale"; minimum: 1; maximum: 1.25; step: .01; value: root.style.hoverScale || 1 }
        AppCheckBox { text: qsTr("Automatic text contrast"); checked: root.style.autoContrast || false; onClicked: editor.setStyleField("autoContrast", checked) }
        Repeater {
            model: [
                {name: qsTr("Slices"), key: "sectorColor"}, {name: qsTr("Highlight"), key: "hoverColor"},
                {name: qsTr("Text"), key: "textColor"}, {name: qsTr("Center"), key: "centerColor"},
                {name: qsTr("Border"), key: "borderColor"}, {name: qsTr("Submenu"), key: "backgroundColor"}
            ]
            RowLayout {
                required property var modelData
                Layout.fillWidth: true
                Label { text: modelData.name; font.pixelSize: 14; color: T.ink; Layout.fillWidth: true }
                AppButton {
                    kind: "secondary"; implicitWidth: 64
                    hint: qsTr("Change %1 color").arg(modelData.name.toLowerCase())
                    onClicked: appController.chooseColor(modelData.key)
                    Rectangle { width: 28; height: 18; anchors.centerIn: parent; color: root.style[modelData.key] || "transparent"; radius: 4; border.color: T.line }
                }
            }
        }
    }
}
