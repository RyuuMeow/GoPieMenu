import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

Popup {
    id: root
    property string title: ""
    property bool dismissible: true
    property Component footer
    property int bodySpacing: 16
    default property alias body: bodyColumn.data
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(480, parent.width - 32)
    implicitHeight: titleRow.implicitHeight + bodyColumn.implicitHeight + (footerLoader.active ? footerLoader.implicitHeight + 16 : 0) + 64
    height: Math.min(implicitHeight, parent.height - 32)
    padding: 24
    modal: true; focus: true
    popupType: Popup.Item
    closePolicy: dismissible ? Popup.CloseOnEscape : Popup.NoAutoClose
    Connections {
        target: root.parent
        function onWidthChanged() { if (root.visible) Qt.callLater(function() { root.contentItem.forceActiveFocus() }) }
        function onHeightChanged() { if (root.visible) Qt.callLater(function() { root.contentItem.forceActiveFocus() }) }
    }
    background: Rectangle { color: T.surface; border.color: T.line; radius: 14 }
    Overlay.modal: Rectangle { color: T.scrim }
    contentItem: ColumnLayout {
        spacing: 16
        RowLayout {
            id: titleRow
            Layout.fillWidth: true
            Label { text: root.title; color: T.ink; font.pixelSize: 19; font.weight: Font.DemiBold; Layout.fillWidth: true }
            AppButton { visible: root.dismissible; iconName: "cancel.svg"; hint: "Close"; compact: true; onClicked: root.close() }
        }
        ScrollView {
            id: scroll
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: availableWidth
            contentHeight: bodyColumn.implicitHeight
            rightPadding: 12
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: contentHeight > availableHeight + 1 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            ColumnLayout { id: bodyColumn; width: scroll.availableWidth; spacing: root.bodySpacing }
        }
        Loader { id: footerLoader; Layout.fillWidth: true; active: root.footer !== null; visible: active; sourceComponent: root.footer }
    }
}
