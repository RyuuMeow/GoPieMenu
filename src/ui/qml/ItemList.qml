import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

Rectangle {
    id: root
    color: "#fafbfd"
    signal closeList()
    implicitHeight: 200
    property string pressedId: ""
    property string draggedName: ""
    property string draggedIcon: ""
    property int sourceIndex: -1
    property int insertionIndex: -1
    property bool dragging: false
    property point pressPoint
    property point pointer
    readonly property int rowHeight: 43
    readonly property int rowSpacing: 3
    readonly property int selectedIndex: {
        const items = editor.items
        for (let i = 0; i < items.length; ++i) if (items[i].id === editor.selectedId) return i
        return -1
    }
    function prepare(item, index, point) {
        pressedId = item.id; draggedName = item.name; draggedIcon = item.icon
        sourceIndex = index; pressPoint = point; pointer = point
    }
    function updateInsertion() {
        const p = viewport.mapToItem(list, pointer.x, pointer.y)
        insertionIndex = pointer.x < 0 || pointer.x > viewport.width || pointer.y < 0 || pointer.y > viewport.height
            ? -1 : Math.max(0, Math.min(list.count, Math.floor((p.y + list.contentY + rowHeight / 2) / (rowHeight + rowSpacing))))
    }
    function track(point) {
        if (!pressedId.length) return
        pointer = point
        if (!dragging && Math.hypot(point.x - pressPoint.x, point.y - pressPoint.y) >= Qt.styleHints.startDragDistance) dragging = true
        if (dragging) updateInsertion()
    }
    function cancelDrag() {
        dragging = false; pressedId = ""; sourceIndex = -1; insertionIndex = -1
    }
    function finish() {
        const id = pressedId
        const wasDragging = dragging
        const from = sourceIndex
        const gap = insertionIndex
        cancelDrag()
        if (!id.length) return
        if (!wasDragging) editor.selectItem(id)
        else if (gap >= 0) {
            // moveItem accepts the final index after removing the source row.
            const target = gap > from ? gap - 1 : gap
            if (target !== from) editor.moveItem(id, editor.folderId, target)
        }
    }
    onVisibleChanged: if (!visible) cancelDrag()
    Shortcut { sequence: "Escape"; enabled: root.dragging; onActivated: root.cancelDrag() }
    Timer {
        interval: 30; repeat: true; running: root.dragging
        onTriggered: {
            const maxY = Math.max(0, list.contentHeight - list.height)
            if (root.pointer.y >= 0 && root.pointer.y < 30) list.contentY = Math.max(0, list.contentY - 10)
            else if (root.pointer.y > viewport.height - 30 && root.pointer.y <= viewport.height) list.contentY = Math.min(maxY, list.contentY + 10)
            root.updateInsertion()
        }
    }
    Rectangle { width: parent.width; height: 1; color: T.line }
    ColumnLayout {
        anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 16
        spacing: 0
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 5
            Label { text: "Arrange actions"; font.pixelSize: 13; color: T.muted; Layout.fillWidth: true }
            AppButton { iconName: "nav-arrow-up.svg"; compact: true; hint: "Move selected action up"; enabled: root.selectedIndex > 0; onClicked: editor.moveSelected(-1) }
            AppButton { iconName: "nav-arrow-down.svg"; compact: true; hint: "Move selected action down"; enabled: root.selectedIndex >= 0 && root.selectedIndex < list.count - 1; onClicked: editor.moveSelected(1) }
            AppButton { iconName: "cancel.svg"; compact: true; hint: "Close list"; onClicked: root.closeList() }
        }
        Item {
            id: viewport
            objectName: "arrangeViewport"
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            ListView {
                id: list
                objectName: "arrangeList"
                anchors.fill: parent; anchors.topMargin: 5; anchors.bottomMargin: 5
                clip: true; spacing: root.rowSpacing
                model: editor.items
                interactive: !root.dragging
                ScrollBar.vertical: ScrollBar {}
                delegate: Rectangle {
                    id: row
                    objectName: "arrange-row-" + modelData.id
                    required property var modelData
                    required property int index
                    width: list.width; height: root.rowHeight; radius: 7
                    opacity: root.dragging && root.pressedId === modelData.id ? .3 : 1
                    color: editor.selectedId === modelData.id ? T.tint : pointerArea.containsMouse ? T.hover : "transparent"
                    MouseArea {
                        id: pointerArea
                        anchors.fill: parent
                        hoverEnabled: true
                        preventStealing: true
                        cursorShape: root.dragging ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        onPressed: function(mouse) { root.prepare(row.modelData, row.index, mapToItem(viewport, mouse.x, mouse.y)) }
                        onPositionChanged: function(mouse) { if (pressed) root.track(mapToItem(viewport, mouse.x, mouse.y)) }
                        onReleased: root.finish()
                        onCanceled: root.cancelDrag()
                    }
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 9; anchors.rightMargin: 8; spacing: 10
                        Item {
                            objectName: "drag-handle-" + row.modelData.id
                            width: 24; Layout.fillHeight: true
                            IconImage { name: "move-cross.svg"; width: 17; height: 17; anchors.centerIn: parent; tint: T.muted }
                        }
                        IconImage { name: row.modelData.icon; width: 19; height: 19 }
                        Label { text: row.modelData.name; Layout.fillWidth: true; Layout.minimumWidth: 0; color: T.ink; font.pixelSize: 14; elide: Text.ElideRight }
                        Label { visible: row.modelData.action === 6; text: row.modelData.childCount; color: T.muted; font.pixelSize: 12 }
                        AppButton { visible: row.modelData.action === 6; iconName: "nav-arrow-right.svg"; compact: true; hint: "Open submenu"; onClicked: editor.enterFolder(row.modelData.id) }
                    }
                    Accessible.role: Accessible.ListItem
                    Accessible.name: modelData.name
                    Accessible.onPressAction: editor.selectItem(row.modelData.id)
                }
            }
            Rectangle {
                objectName: "dragPreview"
                visible: root.dragging
                x: 12; y: Math.max(0, Math.min(viewport.height - height, root.pointer.y - height / 2))
                width: viewport.width - 24; height: root.rowHeight
                radius: 7; color: T.surface; border.color: T.accent; opacity: .92
                RowLayout {
                    anchors.fill: parent; anchors.margins: 10
                    IconImage { name: root.draggedIcon; width: 19; height: 19 }
                    Label { text: root.draggedName; color: T.ink; Layout.fillWidth: true; elide: Text.ElideRight }
                }
            }
            Rectangle {
                objectName: "insertionLine"
                visible: root.dragging && root.insertionIndex >= 0
                x: 2; width: viewport.width - 4; height: 2; radius: 1
                color: T.accent
                y: Math.max(1, Math.min(viewport.height - 3, list.y + root.insertionIndex * (root.rowHeight + root.rowSpacing) - list.contentY - root.rowSpacing / 2))
            }
        }
    }
}
