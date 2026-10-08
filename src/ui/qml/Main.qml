import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GoPieMenu 1.0
import "Theme.js" as T

ApplicationWindow {
    id: root
    objectName: "editorWindow"
    width: 1160; height: 760
    minimumWidth: 780; minimumHeight: 480
    visible: false
    title: "GoPieMenu"
    color: T.background
    font.family: "Segoe UI"
    font.pixelSize: 14
    palette.window: T.background
    palette.base: T.surface
    palette.text: T.ink
    palette.windowText: T.ink
    palette.buttonText: T.ink
    palette.button: T.surface
    palette.highlight: T.accent
    palette.highlightedText: "#ffffff"
    property string panelMode: ""
    property bool compactMode: width < 1000
    property bool listVisible: false
    property string previousSelection: ""
    onClosing: function(event) {
        root.contentItem.forceActiveFocus()
        recorder.cancel()
        event.accepted = appController.previewMode
        if (!event.accepted) root.hide()
    }

    function closePanel() {
        if (panelMode === "item") editor.selectItem("")
        panelMode = ""
    }
    function applyDraft() {
        root.contentItem.forceActiveFocus()
        editor.apply()
    }
    Connections {
        target: editor
        function onSelectionActivated() { root.panelMode = editor.selectedId.length ? "item" : "" }
        function onChanged() {
            if (editor.selectedId !== root.previousSelection) {
                if (editor.selectedId.length) root.panelMode = "item"
                else if (root.panelMode === "item") root.panelMode = ""
                root.previousSelection = editor.selectedId
            }
        }
    }
    Shortcut { sequence: StandardKey.Save; enabled: !recorder.active; onActivated: root.applyDraft() }
    Shortcut { sequences: [StandardKey.Undo]; enabled: !recorder.active; onActivated: editor.undo() }
    Shortcut { sequences: [StandardKey.Redo]; enabled: !recorder.active; onActivated: editor.redo() }

    header: Rectangle {
        height: 68
        color: T.surface
        Rectangle { height: 1; width: parent.width; anchors.bottom: parent.bottom; color: T.line }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20
            spacing: 8
            AppCombo {
                objectName: "profileSelector"
                Layout.preferredWidth: root.width < 900 ? 155 : 215
                model: editor.profiles
                textRole: "name"; valueRole: "id"
                currentIndex: count > 0 ? indexOfValue(editor.profileId) : -1
                onActivated: editor.selectProfile(currentValue)
                Accessible.name: "Current menu"
            }
            AppButton {
                iconName: "more-horiz.svg"; hint: "Manage menus"
                onClicked: profileMenu.open()
                Menu {
                    id: profileMenu
                    y: parent.height + 4
                    MenuItem { text: "New menu"; onTriggered: { editor.addProfile(); root.panelMode = "menu" } }
                    MenuItem { text: "Menu settings"; onTriggered: root.panelMode = "menu" }
                    MenuItem { text: "Duplicate menu"; onTriggered: { editor.duplicateProfile(); root.panelMode = "menu" } }
                    MenuSeparator {}
                    MenuItem { text: "Delete menu"; enabled: editor.profiles.length > 1; onTriggered: editor.removeProfile() }
                }
            }
            Rectangle { Layout.preferredHeight: 24; implicitWidth: 1; color: T.line; Layout.leftMargin: 3; Layout.rightMargin: 3 }
            AppButton {
                objectName: "triggerSummaryButton"
                iconName: "key-command.svg"
                text: root.width < 900 && editor.dirty ? "" : editor.profile.triggerSummary || ""
                hint: "Edit trigger and application scope"
                onClicked: root.panelMode = root.panelMode === "menu" ? "" : "menu"
                checked: root.panelMode === "menu"
            }
            Item { Layout.fillWidth: true }
            AppButton { objectName: "undoButton"; iconName: "undo.svg"; hint: "Undo · Ctrl+Z"; enabled: editor.canUndo; onClicked: editor.undo() }
            AppButton { iconName: "redo.svg"; hint: "Redo · Ctrl+Y"; enabled: editor.canRedo; onClicked: editor.redo() }
            AppButton { visible: editor.dirty; text: "Discard"; onClicked: { root.contentItem.forceActiveFocus(); editor.discard() } }
            AppButton { objectName: "applyButton"; visible: editor.dirty; text: "Apply"; iconName: "check.svg"; kind: "primary"; onClicked: root.applyDraft() }
            AppButton { iconName: "settings.svg"; hint: "Application settings"; onClicked: settings.open() }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            visible: editor.error.length > 0
            Layout.fillWidth: true
            implicitHeight: errorRow.implicitHeight + 22
            color: "#fff2ef"
            RowLayout {
                id: errorRow
                anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 20; anchors.rightMargin: 12
                Label { text: editor.error; color: "#954c41"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                AppButton { iconName: "cancel.svg"; compact: true; hint: "Dismiss message"; onClicked: editor.clearError() }
            }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            ColumnLayout {
                id: canvasColumn
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 24; Layout.rightMargin: 24
                    Layout.topMargin: 20; Layout.bottomMargin: 4
                    spacing: 6
                    AppButton {
                        visible: editor.folderId.length > 0
                        iconName: "arrow-left.svg"; hint: "Back to main menu"
                        onClicked: editor.leaveFolder()
                    }
                    Label {
                        visible: editor.folderId.length > 0
                        text: editor.folderName; font.pixelSize: 15; color: T.ink; elide: Text.ElideRight
                        Layout.maximumWidth: 160
                    }
                    AppButton {
                        objectName: "addActionButton"
                        iconName: "plus.svg"; text: "Add action"; kind: "secondary"
                        onClicked: addMenu.open()
                        Menu {
                            id: addMenu
                            y: parent.height + 4
                            MenuItem { text: "Keyboard shortcut"; onTriggered: editor.addItem(3) }
                            MenuItem { text: "Open application"; onTriggered: editor.addItem(1) }
                            MenuItem { text: "Open file or folder"; onTriggered: editor.addItem(4) }
                            MenuItem { text: "Open website"; onTriggered: editor.addItem(5) }
                            MenuItem { text: "Run command"; onTriggered: editor.addItem(2) }
                            MenuSeparator { visible: editor.folderId.length === 0 }
                            MenuItem { text: "New submenu"; visible: editor.folderId.length === 0; onTriggered: editor.addItem(6) }
                        }
                    }
                    Item { Layout.fillWidth: true }
                    AppButton {
                        objectName: "arrangeButton"
                        iconName: "list.svg"; hint: "Arrange actions"; checked: root.listVisible
                        onClicked: root.listVisible = !root.listVisible
                    }
                    AppButton {
                        objectName: "appearanceButton"
                        iconName: "palette.svg"; text: canvasColumn.width > 680 ? "Appearance" : ""
                        hint: "Appearance"; checked: root.panelMode === "appearance"
                        onClicked: root.panelMode = root.panelMode === "appearance" ? "" : "appearance"
                    }
                }
                Item {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    PiePreview {
                        id: preview
                        objectName: "pieCanvas"
                        anchors.fill: parent
                        session: editor
                        icons: iconService
                        visible: editor.items.length > 0
                        ToolTip.visible: hoveredName.length > 0
                        ToolTip.text: hoveredName
                        ToolTip.delay: 1000
                    }
                    ColumnLayout {
                        visible: editor.items.length === 0
                        anchors.centerIn: parent
                        spacing: 14
                        IconImage { name: "add-circle.svg"; width: 40; height: 40; tint: "#91a7c5"; Layout.alignment: Qt.AlignHCenter }
                        Label { text: editor.folderId.length ? "Add an action to this submenu" : "Build your menu"; color: T.ink; font.pixelSize: 20; font.weight: Font.DemiBold; Layout.alignment: Qt.AlignHCenter }
                        Label { text: "Start with a shortcut, an app, or a website."; color: T.muted; font.pixelSize: 14; Layout.alignment: Qt.AlignHCenter }
                        AppButton { text: "Add action"; iconName: "plus.svg"; kind: "primary"; Layout.alignment: Qt.AlignHCenter; onClicked: addMenu.open() }
                    }
                    DropArea {
                        anchors.fill: parent
                        onDropped: function(drop) {
                            if (drop.hasUrls) { appController.addDroppedFiles(drop.urls); drop.acceptProposedAction() }
                        }
                    }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.bottomMargin: 18
                    spacing: 8
                    Label {
                        text: !editor.profile.enabled ? "This menu is disabled" : editor.selectedId.length ? "Changes stay in preview until you apply." : editor.folderId.length ? "Click an action to edit" : "Click a slice to edit"
                        color: T.muted; font.pixelSize: 13
                    }
                    AppButton { visible: !editor.profile.enabled && editor.items.length > 0; text: "Enable"; compact: true; onClicked: editor.setProfileField("enabled", true) }
                }
                ItemList {
                    visible: root.listVisible
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(220, root.height * .28)
                    onCloseList: root.listVisible = false
                }
            }
            Rectangle { visible: !root.compactMode && root.panelMode.length > 0; Layout.fillHeight: true; implicitWidth: 1; color: T.line }
            InspectorPanel {
                objectName: "wideInspector"
                visible: !root.compactMode && root.panelMode.length > 0
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                mode: visible ? root.panelMode : ""
                onClosePanel: root.closePanel()
                onChooseIcon: iconPicker.open()
            }
        }
    }

    Drawer {
        id: compactInspector
        objectName: "compactDrawer"
        edge: Qt.RightEdge
        width: Math.min(340, root.width - 48)
        y: root.header.height
        height: root.height - y
        modal: false
        interactive: false
        visible: root.compactMode && root.panelMode.length > 0
        padding: 0
        closePolicy: Popup.NoAutoClose
        background: Rectangle { color: T.surface; border.color: T.line }
        contentItem: InspectorPanel {
            mode: compactInspector.visible ? root.panelMode : ""
            onClosePanel: root.closePanel()
            onChooseIcon: iconPicker.open()
        }
    }
    IconPicker { id: iconPicker; parent: Overlay.overlay }
    Popup {
        id: settings
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(440, root.width - 32)
        implicitHeight: settingsContent.implicitHeight + 40
        padding: 20
        modal: true; focus: true
        background: Rectangle { color: T.surface; border.color: T.line; radius: 14 }
        Overlay.modal: Rectangle { color: "#25344920" }
        ColumnLayout {
            id: settingsContent
            width: parent.width
            spacing: 14
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Settings"; color: T.ink; font.pixelSize: 19; font.weight: Font.DemiBold; Layout.fillWidth: true }
                AppButton { iconName: "cancel.svg"; hint: "Close settings"; onClicked: settings.close() }
            }
            CheckBox { text: "Start with Windows"; font.pixelSize: 14; checked: editor.startWithWindows; onClicked: editor.setStartWithWindows(checked) }
            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: T.line }
            RowLayout {
                AppButton { text: "Import"; iconName: "import.svg"; kind: "secondary"; onClicked: { settings.close(); appController.importConfig() } }
                AppButton { text: "Export"; iconName: "upload.svg"; kind: "secondary"; onClicked: appController.exportConfig() }
            }
            AppButton { text: "Open custom icon folder"; iconName: "folder.svg"; onClicked: appController.openIconDirectory() }
            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: T.line }
            AppButton { text: "About"; onClicked: about.open() }
        }
    }
    Popup {
        id: about
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 320; padding: 24; implicitHeight: aboutContent.implicitHeight + 48
        modal: true; focus: true
        background: Rectangle { color: T.surface; border.color: T.line; radius: 14 }
        ColumnLayout {
            id: aboutContent
            width: parent.width; spacing: 12
            Label { text: "GoPieMenu"; font.pixelSize: 22; font.weight: Font.DemiBold; color: T.ink }
            Label { text: "Version " + appController.version; color: T.muted }
            Label { text: "GoPieMenu · GPL-3.0\nIcons by Iconoir · MIT"; color: T.muted; font.pixelSize: 13 }
            AppButton { text: "Close"; kind: "secondary"; onClicked: about.close() }
        }
    }
}
