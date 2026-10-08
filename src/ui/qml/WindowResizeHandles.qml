import QtQuick
import QtQuick.Window

Item {
    id: root
    required property var targetWindow
    visible: !appController.maximized && targetWindow.visibility !== Window.FullScreen
    Repeater {
        model: [Qt.LeftEdge, Qt.RightEdge, Qt.TopEdge, Qt.BottomEdge,
            Qt.LeftEdge | Qt.TopEdge, Qt.RightEdge | Qt.TopEdge, Qt.LeftEdge | Qt.BottomEdge, Qt.RightEdge | Qt.BottomEdge]
        MouseArea {
            required property int modelData
            readonly property bool edgeLeft: (modelData & Qt.LeftEdge) !== 0
            readonly property bool edgeRight: (modelData & Qt.RightEdge) !== 0
            readonly property bool edgeTop: (modelData & Qt.TopEdge) !== 0
            readonly property bool edgeBottom: (modelData & Qt.BottomEdge) !== 0
            readonly property bool corner: (edgeLeft || edgeRight) && (edgeTop || edgeBottom)
            x: edgeRight ? root.width - width : 0
            y: edgeBottom ? root.height - height : 0
            width: corner ? 8 : edgeLeft || edgeRight ? 5 : root.width
            height: corner ? 8 : edgeTop || edgeBottom ? 5 : root.height
            z: corner ? 1 : 0
            cursorShape: corner ? (edgeLeft === edgeTop ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor) : edgeLeft || edgeRight ? Qt.SizeHorCursor : Qt.SizeVerCursor
            onPressed: root.targetWindow.startSystemResize(modelData)
        }
    }
}
