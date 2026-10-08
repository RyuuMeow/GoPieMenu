import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

Popup {
    id: root
    objectName: "iconPicker"
    width: Math.min(520, parent.width - 32)
    height: Math.min(540, parent.height - 40)
    anchors.centerIn: parent
    modal: true
    focus: true
    padding: 20
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    background: Rectangle { color: T.surface; border.color: T.line; radius: 14 }
    Overlay.modal: Rectangle { color: "#25344920" }
    onOpened: { search.text = ""; iconCatalog.query = ""; search.forceActiveFocus() }
    onClosed: { searchDelay.stop(); iconCatalog.query = "" }
    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        RowLayout {
            Layout.fillWidth: true
            AppField {
                id: search
                objectName: "iconSearch"
                Layout.fillWidth: true
                placeholderText: "Search icons"
                onTextEdited: searchDelay.restart()
                Accessible.name: "Search icons"
            }
            AppButton { iconName: "cancel.svg"; hint: "Close"; onClicked: root.close() }
        }
        Timer { id: searchDelay; interval: 16; onTriggered: iconCatalog.query = search.text }
        GridView {
            id: grid
            objectName: "iconGrid"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: width / Math.max(3, Math.floor(width / 80))
            cellHeight: 88
            cacheBuffer: cellHeight * 2
            model: root.visible ? iconCatalog : null
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: Item {
                id: cell
                required property string iconId
                required property string displayName
                width: grid.cellWidth; height: grid.cellHeight
                Rectangle { anchors.fill: parent; anchors.margins: 3; color: hover.hovered ? T.tint : "transparent"; radius: 8 }
                IconImage { name: cell.iconId; width: 30; height: 30; anchors.horizontalCenter: parent.horizontalCenter; y: 12 }
                Text {
                    text: cell.displayName; font.pixelSize: 11; color: T.muted; elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 5; y: 54
                }
                HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
                TapHandler { onTapped: { editor.setItemField("icon", cell.iconId); root.close() } }
                ToolTip.visible: hover.hovered
                ToolTip.text: cell.displayName
                ToolTip.delay: 700
                Accessible.role: Accessible.Button
                Accessible.name: cell.displayName
                Accessible.onPressAction: { editor.setItemField("icon", cell.iconId); root.close() }
            }
            Label {
                anchors.centerIn: parent
                visible: grid.count === 0
                text: iconCatalog.loading ? "Loading icons…" : "No matching icons"
                color: T.muted; font.pixelSize: 14
            }
        }
        RowLayout {
            Layout.fillWidth: true
            AppButton { text: "Clear icon"; onClicked: { editor.setItemField("icon", ""); root.close() } }
            Item { Layout.fillWidth: true }
            BusyIndicator { running: iconCatalog.loading; visible: running; implicitWidth: 22; implicitHeight: 22 }
            Label { text: iconCatalog.count + " icons"; color: T.muted; font.pixelSize: 12 }
        }
    }
}
