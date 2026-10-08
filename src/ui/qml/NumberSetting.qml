import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as T
ColumnLayout {
    id: root
    property string label: ""
    property string field: ""
    property real minimum: 0
    property real maximum: 100
    property real step: 1
    property real value: 0
    property string suffix: ""
    spacing: 3
    RowLayout {
        Layout.fillWidth: true
        Label { text: root.label; color: T.ink; font.pixelSize: 14; Layout.fillWidth: true }
        Label { text: (root.step < 1 ? root.value.toFixed(2) : Math.round(root.value)) + root.suffix; color: T.muted; font.pixelSize: 13 }
    }
    AppSlider {
        Layout.fillWidth: true
        from: root.minimum; to: root.maximum; stepSize: root.step
        value: root.value
        onMoved: editor.setStyleField(root.field, value)
        Accessible.name: root.label
    }
}
