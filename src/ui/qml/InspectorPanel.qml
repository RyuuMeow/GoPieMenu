import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

Rectangle {
    id: root
    property string mode: ""
    signal closePanel()
    signal chooseIcon()
    color: T.surface
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 22; Layout.rightMargin: 14
            Layout.topMargin: 14; Layout.bottomMargin: 8
            Label {
                text: root.mode === "appearance" ? "Appearance" : root.mode === "menu" ? "Menu settings" : ""
                font.pixelSize: 17; font.weight: Font.DemiBold; color: T.ink
                Layout.fillWidth: true
            }
            AppButton { iconName: "cancel.svg"; hint: "Close panel"; compact: true; onClicked: root.closePanel() }
        }
        ScrollView {
            id: scroll
            objectName: "inspectorScroll"
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: availableWidth
            contentHeight: loader.height + 32
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            Loader {
                id: loader
                x: 22; y: 8
                width: scroll.availableWidth - 44
                sourceComponent: root.mode === "appearance" ? appearance : root.mode === "menu" ? menuSettings : root.mode === "item" ? itemEditor : null
            }
        }
    }
    Component { id: itemEditor; ItemInspector { onChooseIcon: root.chooseIcon() } }
    Component { id: appearance; AppearancePane {} }
    Component { id: menuSettings; MenuSettingsPane {} }
}
