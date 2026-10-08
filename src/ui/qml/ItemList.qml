import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

Rectangle {
    id: root
    color: "#fafbfd"
    signal closeList()
    implicitHeight: 200
    Rectangle { width: parent.width; height: 1; color: T.line }
    ColumnLayout {
        anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 16
        spacing: 0
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 5
            Label { text: "Arrange actions"; font.pixelSize: 13; color: T.muted; Layout.fillWidth: true }
            AppButton { iconName: "nav-arrow-up.svg"; compact: true; hint: "Move selected action up"; enabled: editor.selectedId.length > 0; onClicked: editor.moveSelected(-1) }
            AppButton { iconName: "nav-arrow-down.svg"; compact: true; hint: "Move selected action down"; enabled: editor.selectedId.length > 0; onClicked: editor.moveSelected(1) }
            AppButton { iconName: "cancel.svg"; compact: true; hint: "Close list"; onClicked: root.closeList() }
        }
        ListView {
            id: list
            objectName: "arrangeList"
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true; spacing: 3
            model: editor.items
            ScrollBar.vertical: ScrollBar {}
            delegate: Rectangle {
                id: row
                objectName: "arrange-row-" + modelData.id
                required property var modelData
                required property int index
                width: list.width; height: 43; radius: 7
                color: drop.containsDrag ? "#dfebfc" : editor.selectedId === modelData.id ? T.tint : hover.hovered ? "#eff2f7" : "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 9; anchors.rightMargin: 8; spacing: 10
                    Item {
                        width: 24; Layout.fillHeight: true
                        IconImage { name: "drag.svg"; width: 17; height: 17; anchors.centerIn: parent; tint: T.muted }
                        Item {
                            id: dragProxy
                            property string itemId: row.modelData.id
                            width: 1; height: 1
                            Drag.active: dragArea.drag.active
                            Drag.keys: ["gpm-item"]
                            Drag.source: dragProxy
                        }
                        MouseArea {
                            id: dragArea
                            objectName: "drag-handle-" + row.modelData.id
                            anchors.fill: parent
                            cursorShape: Qt.SizeAllCursor
                            preventStealing: true
                            drag.target: dragProxy
                            drag.axis: Drag.YAxis
                            drag.smoothed: false
                            onReleased: { dragProxy.Drag.drop(); dragProxy.x = 0; dragProxy.y = 0 }
                        }
                    }
                    IconImage { name: row.modelData.icon; width: 19; height: 19 }
                    Label {
                        text: row.modelData.name
                        Layout.fillWidth: true
                        color: T.ink; font.pixelSize: 14; elide: Text.ElideRight
                        TapHandler { onTapped: editor.selectItem(row.modelData.id) }
                    }
                    Label { visible: row.modelData.action === 6; text: row.modelData.childCount; color: T.muted; font.pixelSize: 12 }
                    AppButton { visible: row.modelData.action === 6; iconName: "nav-arrow-right.svg"; compact: true; hint: "Open submenu"; onClicked: editor.enterFolder(row.modelData.id) }
                }
                HoverHandler { id: hover }
                DropArea {
                    id: drop
                    anchors.fill: parent
                    keys: ["gpm-item"]
                    onDropped: function(event) {
                        const id = event.source.itemId
                        event.acceptProposedAction()
                        editor.moveItem(id, editor.folderId, row.index)
                    }
                }
                Accessible.role: Accessible.ListItem
                Accessible.name: modelData.name
                Accessible.onPressAction: editor.selectItem(row.modelData.id)
            }
        }
    }
}
