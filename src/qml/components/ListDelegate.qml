import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import "../theme"

ItemDelegate {
    id: delegate
    leftInset: 8
    rightInset: leftInset

    default property alias children: rowLayout.children

    background: Rectangle {
        color: delegate.down ? Theme.contactPressedColor : "Transparent"
        radius: 6
    }

    contentItem: RowLayout {
        id: rowLayout
        anchors.leftMargin: 2 * spacing
        anchors.rightMargin: anchors.leftMargin
        anchors.fill: parent
        height: Theme.avatarHeight
        spacing: 10
    }
}
