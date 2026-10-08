import QtQuick
import QtQuick.Controls
import "Theme.js" as T
ComboBox {
    id: root
    property var selectionValue: undefined
    function syncSelection() {
        if (selectionValue !== undefined) currentIndex = indexOfValue(selectionValue)
    }
    onSelectionValueChanged: Qt.callLater(syncSelection)
    onModelChanged: Qt.callLater(syncSelection)
    onCountChanged: Qt.callLater(syncSelection)
    implicitHeight: 42
    implicitWidth: 200
    font.pixelSize: 14
    leftPadding: 12
    rightPadding: 34
    contentItem: Text {
        text: root.displayText
        font: root.font
        color: root.enabled ? T.ink : T.muted
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: IconImage {
        name: "nav-arrow-down.svg"
        width: 16; height: 16
        x: root.width - width - 12
        y: (root.height - height) / 2
        tint: T.muted
    }
    background: Rectangle {
        color: root.hovered ? "#f9fbfe" : T.surface
        radius: 8
        border.color: root.visualFocus ? T.accent : T.line
    }
    popup: Popup {
        y: root.height + 5
        width: root.width
        padding: 5
        implicitHeight: Math.min(300, contentItem.implicitHeight + 10)
        background: Rectangle { color: T.surface; radius: 9; border.color: T.line }
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
    delegate: ItemDelegate {
        width: root.width - 10
        implicitHeight: 40
        text: root.textAt(index)
        font.pixelSize: 14
        highlighted: root.highlightedIndex === index
        contentItem: Text { text: parent.text; color: T.ink; font: parent.font; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
        background: Rectangle { radius: 6; color: parent.highlighted ? T.hover : "transparent" }
    }
}
