import QtQuick
import QtQuick.Controls
import "Theme.js" as T

MenuSeparator {
    leftPadding: 8; rightPadding: 8
    topPadding: 5; bottomPadding: 5
    contentItem: Rectangle { implicitHeight: 1; color: T.line }
}
