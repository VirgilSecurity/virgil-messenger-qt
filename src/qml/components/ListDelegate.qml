import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import "../theme"

ItemDelegate {
    id: delegate

    default property alias children: rowLayout.children

    background: Rectangle {
        color: delegate.down ? Theme.contactPressedColor : "Transparent"
        radius: 6
    }

    contentItem: RowLayout {
        id: rowLayout
        anchors {
            fill: parent
            leftMargin: Theme.smallSpacing
            rightMargin: Theme.smallSpacing
        }
        height: Theme.avatarHeight
        spacing: Theme.smallSpacing
    }
}
