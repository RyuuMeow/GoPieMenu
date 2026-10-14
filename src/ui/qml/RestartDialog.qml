import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

AppDialog {
    id: root
    objectName: "restartDialog"
    title: qsTr("Restart required")
    Label {
        Layout.fillWidth: true
        text: qsTr("Restart GoPieMenu to use the new language. Restarting will apply all pending changes.")
        wrapMode: Text.WordWrap; color: T.ink
    }
    Label {
        Layout.fillWidth: true
        text: qsTr("Choose Later to keep editing, then apply your changes before restarting manually.")
        wrapMode: Text.WordWrap; color: T.muted; font.pixelSize: 12
    }
    Label {
        Layout.fillWidth: true
        visible: editor.error.length > 0
        text: editor.error; wrapMode: Text.WordWrap; color: T.danger
    }
    footer: RowLayout {
        Item { Layout.fillWidth: true }
        AppButton { objectName: "restartLaterButton"; text: qsTr("Later"); onClicked: root.close() }
        AppButton {
            objectName: "restartNowButton"
            text: qsTr("Restart"); kind: "primary"
            onClicked: if (appController.restartForLanguage()) root.close()
        }
    }
}
