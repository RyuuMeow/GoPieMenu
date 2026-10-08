import QtQuick
import QtQuick.Controls
import "Theme.js" as T

Slider {
    id: root
    implicitHeight: 32
    background: Rectangle {
        x: root.leftPadding; y: root.topPadding + root.availableHeight/2 - height/2
        width: root.availableWidth; height: 4; radius: 2; color: T.line
        Rectangle { width: root.visualPosition * parent.width; height: parent.height; radius: 2; color: T.accent }
    }
    handle: Rectangle {
        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding + root.availableHeight/2 - height/2
        width: 16; height: 16; radius: 8
        color: root.pressed ? T.tint : T.surface; border.color: T.accent; border.width: root.visualFocus ? 2 : 1
    }
}
