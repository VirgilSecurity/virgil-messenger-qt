import QtQuick 2.15

import "../theme"

Rectangle {
    implicitHeight: Theme.headerHeight
    color: "transparent"

    property alias showSeparator: separator.visible

    HorizontalRule {
        id: separator
        anchors.leftMargin: Theme.margin
        anchors.rightMargin: Theme.margin
        anchors.bottom: parent.bottom
    }
}
