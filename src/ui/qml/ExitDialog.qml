import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

AppDialog {
    id: root
    objectName: "exitDialog"
    title: qsTr("Unsaved changes")
    onClosed: appController.cancelExit()
    Label { Layout.fillWidth: true; text: qsTr("Apply your changes before quitting?"); wrapMode: Text.WordWrap; color: T.ink; font.pixelSize: 14 }
    Label { Layout.fillWidth: true; visible: editor.error.length > 0; text: editor.error; wrapMode: Text.WordWrap; color: T.danger; font.pixelSize: 13 }
    footer: RowLayout {
        AppButton { objectName: "exitDiscardButton"; text: qsTr("Discard"); kind: "danger"; onClicked: if (appController.resolveExit("discard")) root.close() }
        Item { Layout.fillWidth: true }
        AppButton { objectName: "exitCancelButton"; text: qsTr("Cancel"); onClicked: root.close() }
        AppButton { objectName: "exitApplyButton"; text: qsTr("Apply & quit"); kind: "primary"; onClicked: if (appController.resolveExit("apply")) root.close() }
    }
}
