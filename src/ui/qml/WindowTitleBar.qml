import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

Rectangle {
    id: root
    required property var targetWindow
    implicitHeight: 40
    color: T.surface
    function toggleMaximized() { appController.toggleMaximized() }
    Item {
        anchors.fill: parent; anchors.rightMargin: 138
        DragHandler { target: null; onActiveChanged: if (active) root.targetWindow.startSystemMove() }
        TapHandler { onDoubleTapped: root.toggleMaximized() }
        Text { anchors.left: parent.left; anchors.leftMargin: 20; anchors.verticalCenter: parent.verticalCenter; text: root.targetWindow.title; color: T.muted; font.pixelSize: 12 }
    }
    Row {
        anchors.right: parent.right; anchors.rightMargin: 6; anchors.verticalCenter: parent.verticalCenter
        WindowControlButton { objectName: "windowMinimizeButton"; iconName: "window-minimize.svg"; hint: "Minimize"; onClicked: root.targetWindow.showMinimized() }
        WindowControlButton { objectName: "windowMaximizeButton"; iconName: appController.maximized ? "window-restore.svg" : "window-maximize.svg"; hint: appController.maximized ? "Restore" : "Maximize"; onClicked: root.toggleMaximized() }
        WindowControlButton { objectName: "windowCloseButton"; iconName: "window-close.svg"; hint: "Close window"; onClicked: root.targetWindow.close() }
    }
}
