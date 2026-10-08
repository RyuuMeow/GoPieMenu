import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root
    property string label
    property real value
    property int maximum: 100
    signal edited(real value)
    spacing: 10
    FieldLabel { text: root.label; Layout.preferredWidth: 14 }
    AppSlider { objectName: "color" + root.label + "Slider"; Layout.fillWidth: true; from: 0; to: root.maximum; stepSize: 1; value: root.value; onMoved: root.edited(value) }
    AppField {
        objectName: "color" + root.label + "Value"
        Layout.preferredWidth: 52
        implicitHeight: 34; padding: 6
        text: Math.round(root.value)
        horizontalAlignment: Text.AlignHCenter
        validator: IntValidator { bottom: 0; top: root.maximum }
        onEditingFinished: { if (acceptableInput) root.edited(Number(text)); text = Qt.binding(function() { return Math.round(root.value) }) }
    }
}
