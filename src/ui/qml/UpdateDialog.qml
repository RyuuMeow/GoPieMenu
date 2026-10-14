import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

AppDialog {
    id: root
    objectName: "updateDialog"
    readonly property var updater: appController.updater
    title: qsTr("Update available")
    width: Math.min(640, parent.width - 32)
    Label {
        Layout.fillWidth: true
        text: qsTr("GoPieMenu %1 → %2").arg(appController.version).arg(root.updater ? root.updater.version : "")
        color: T.ink; font.pixelSize: 16; font.weight: Font.DemiBold
        wrapMode: Text.WordWrap
    }
    TextArea {
        objectName: "updateReleaseNotes"
        Layout.fillWidth: true
        readOnly: true; selectByMouse: true
        textFormat: TextEdit.MarkdownText
        text: root.updater && root.updater.notes.length ? root.updater.notes : qsTr("Release notes are unavailable. You can view this release on GitHub.")
        wrapMode: TextEdit.Wrap
        color: T.ink; font.pixelSize: 14
        padding: 0; background: null
        onLinkActivated: function(link) { appController.openReleaseLink(link) }
    }
    Label {
        Layout.fillWidth: true
        visible: root.updater && !root.updater.canDownload
        text: qsTr("This release has no verified Windows installer. View the release on GitHub.")
        color: T.muted; wrapMode: Text.WordWrap
    }
    ProgressBar {
        objectName: "updateProgress"
        Layout.fillWidth: true
        visible: root.updater && root.updater.downloading
        indeterminate: value === 0
        value: root.updater ? root.updater.progress : 0
    }
    Label {
        Layout.fillWidth: true
        visible: root.updater && root.updater.downloading
        text: qsTr("Downloading and verifying the installer… %1%").arg(Math.round(root.updater ? root.updater.progress * 100 : 0))
        color: T.muted; wrapMode: Text.WordWrap
    }
    Label {
        objectName: "updateReadyMessage"
        Layout.fillWidth: true
        visible: root.updater && root.updater.ready
        text: qsTr("The installer is verified. Install now? GoPieMenu will close after you resolve any unsaved changes.")
        color: T.ink; wrapMode: Text.WordWrap
    }
    Label {
        Layout.fillWidth: true
        visible: root.updater && root.updater.error.length > 0
        text: root.updater ? root.updater.error : ""
        color: T.danger; wrapMode: Text.WordWrap
    }
    AppButton { text: qsTr("View release on GitHub"); kind: "text"; onClicked: appController.openReleasePage() }
    footer: RowLayout {
        AppButton {
            objectName: "skipUpdateButton"
            visible: root.updater && !root.updater.downloading && !root.updater.ready
            text: qsTr("Skip this version")
            onClicked: if (root.updater.skipVersion()) root.close()
        }
        AppButton {
            objectName: "cancelUpdateDownloadButton"
            visible: root.updater && root.updater.downloading
            text: qsTr("Cancel download")
            onClicked: root.updater.cancelDownload()
        }
        Item { Layout.fillWidth: true }
        AppButton { text: qsTr("Later"); onClicked: root.close() }
        AppButton {
            objectName: "downloadUpdateButton"
            visible: root.updater && !root.updater.ready
            enabled: root.updater && root.updater.canDownload && !root.updater.downloading && !root.updater.checking
            text: qsTr("Update"); kind: "primary"
            onClicked: root.updater.download()
        }
        AppButton {
            objectName: "installUpdateButton"
            visible: root.updater && root.updater.ready
            text: qsTr("Install and exit"); kind: "primary"
            onClicked: { root.close(); appController.requestUpdateInstall() }
        }
    }
}
