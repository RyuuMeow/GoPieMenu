import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

ColumnLayout {
    id: root
    property bool advanced: false
    property var style: editor.style
    property bool submenu: editor.folderId.length > 0
    property string previewFolder: editor.folderId
    property string previewProfile: editor.profileId
    onPreviewFolderChanged: submenu = previewFolder.length > 0
    onPreviewProfileChanged: submenu = editor.folderId.length > 0
    function showSubmenu() {
        submenu = true
        if (editor.folderId.length) return
        if (editor.selectedItem.action === 6) {
            editor.enterFolder(editor.selectedId)
            return
        }
        for (var i = 0; i < editor.destinations.length; ++i) {
            if (editor.destinations[i].id.length) {
                editor.enterFolder(editor.destinations[i].id)
                return
            }
        }
    }
    spacing: 14
    Label {
        Layout.fillWidth: true
        text: qsTr("Appearance applies only to this menu.")
        color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
    }
    FieldLabel { text: qsTr("Shared style") }
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
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        AppButton {
            objectName: "pieAppearanceButton"
            Layout.fillWidth: true
            text: qsTr("Pie menu"); kind: "secondary"; checked: !root.submenu
            onClicked: { root.submenu = false; if (editor.folderId.length) editor.leaveFolder() }
        }
        AppButton {
            objectName: "submenuAppearanceButton"
            Layout.fillWidth: true
            text: qsTr("Submenu"); kind: "secondary"; checked: root.submenu
            onClicked: root.showSubmenu()
        }
    }
    Label {
        visible: root.submenu && !editor.folderId.length
        Layout.fillWidth: true
        text: qsTr("Add a submenu to preview its appearance.")
        color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap
    }
    NumberSetting { visible: !root.submenu; Layout.fillWidth: true; label: qsTr("Size"); field: "outerRadius"; minimum: 80; maximum: 400; step: 5; value: root.style.outerRadius || 150 }
    NumberSetting { Layout.fillWidth: true; label: qsTr("Text size"); field: root.submenu ? "submenuFontSize" : "fontSize"; minimum: 8; maximum: 32; value: root.submenu ? root.style.submenuFontSize : root.style.fontSize }
    NumberSetting { Layout.fillWidth: true; label: qsTr("Icon size"); field: root.submenu ? "submenuIconSize" : "iconSize"; minimum: root.submenu ? 8 : 12; maximum: 96; value: root.submenu ? root.style.submenuIconSize : root.style.iconSize }
    NumberSetting { Layout.fillWidth: true; label: qsTr("Text outline"); field: root.submenu ? "submenuTextOutlineThickness" : "textOutlineThickness"; minimum: 0; maximum: 10; step: .5; value: root.submenu ? root.style.submenuTextOutlineThickness : root.style.textOutlineThickness }
    NumberSetting { Layout.fillWidth: true; label: qsTr("Border width"); field: root.submenu ? "submenuBorderWidth" : "borderWidth"; minimum: 0; maximum: 8; step: .5; value: root.submenu ? root.style.submenuBorderWidth : root.style.borderWidth }
    AppButton { text: qsTr("Advanced"); kind: "text"; iconName: root.advanced ? "nav-arrow-down.svg" : "nav-arrow-right.svg"; onClicked: root.advanced = !root.advanced }
    ColumnLayout {
        Layout.fillWidth: true
        visible: root.advanced
        spacing: 16
        NumberSetting { visible: !root.submenu; Layout.fillWidth: true; label: qsTr("Center radius"); field: "innerRadius"; minimum: 10; maximum: Math.min(150, (root.style.outerRadius || 150) - 9); value: root.style.innerRadius || 45 }
        NumberSetting { visible: !root.submenu; Layout.fillWidth: true; label: qsTr("Slice gap"); field: "gapAngle"; minimum: 0; maximum: 20; step: .5; value: root.style.gapAngle || 0; suffix: "°" }
        NumberSetting { visible: !root.submenu; Layout.fillWidth: true; label: qsTr("Hover size"); field: "hoverScale"; minimum: 1; maximum: 1.25; step: .01; value: root.style.hoverScale || 1 }
        Label { Layout.fillWidth: true; text: qsTr("Colors, opacity and animation apply to both the pie and its submenus."); color: T.muted; font.pixelSize: 12; wrapMode: Text.WordWrap }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Opacity"); field: "opacity"; minimum: .1; maximum: 1; step: .05; value: root.style.opacity || .95 }
        NumberSetting { Layout.fillWidth: true; label: qsTr("Animation"); field: "animationDuration"; minimum: 0; maximum: 500; step: 10; value: root.style.animationDuration || 0; suffix: " ms" }
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
