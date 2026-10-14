import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T

AppDialog {
    id: root
    objectName: "runningAppsDialog"
    title: qsTr("Running applications")
    onOpened: { search.text = ""; search.forceActiveFocus() }
    AppField { id: search; objectName: "runningAppsSearch"; Layout.fillWidth: true; placeholderText: qsTr("Search applications"); Accessible.name: qsTr("Search running applications") }
    ListView {
        Layout.fillWidth: true; Layout.preferredHeight: 300
        clip: true; spacing: 3
        model: appController.runningApplications.filter(function(name) { return name.toLowerCase().indexOf(search.text.toLowerCase()) >= 0 })
        ScrollBar.vertical: ScrollBar {}
        delegate: AppMenuItem {
            required property string modelData
            width: ListView.view.width
            text: modelData
            onTriggered: { appController.useRunningApplication(modelData); root.close() }
        }
        Label { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No matching applications"); color: T.muted; font.pixelSize: 13 }
    }
    footer: RowLayout {
        AppButton { text: qsTr("Refresh"); kind: "text"; onClicked: appController.refreshRunningApplications() }
        Item { Layout.fillWidth: true }
        AppButton { text: qsTr("Cancel"); onClicked: root.close() }
    }
}
