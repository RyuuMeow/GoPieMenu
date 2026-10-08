import QtQuick
import "Theme.js" as T

Rectangle {
    id: root
    property color value: "transparent"
    color: "#ffffff"
    radius: 5
    implicitWidth: 32; implicitHeight: 32
    Canvas {
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = "#e2e7ef"
            for (let y = 0; y < height; y += 6)
                for (let x = 0; x < width; x += 6)
                    if ((x/6 + y/6) % 2 === 0) ctx.fillRect(x, y, 6, 6)
        }
    }
    Rectangle { anchors.fill: parent; color: root.value; border.color: T.line; radius: 5 }
}
