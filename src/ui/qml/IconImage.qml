import QtQuick
import QtQuick.Window
Item {
    id: root
    property string name: ""
    property color tint: "#43546d"
    implicitWidth: 20
    implicitHeight: 20
    Image {
        anchors.fill: parent
        source: root.name.length ? "image://icons/" + encodeURIComponent(root.name) + "?color=" + encodeURIComponent(root.tint.toString()) + "&v=" + iconCatalog.revision : ""
        sourceSize: Qt.size(Math.ceil(width * Screen.devicePixelRatio), Math.ceil(height * Screen.devicePixelRatio))
        asynchronous: true
        cache: true
        fillMode: Image.PreserveAspectFit
    }
}
