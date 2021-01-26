import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

ProgressBar {
    id: root
    height: 2

    Behavior on value {
        NumberAnimation { duration: Theme.animationDuration * 2 }
    }

    background: Rectangle {
        implicitWidth: root.width
        implicitHeight: root.height
        radius: height
        color: Theme.menuSeparatorColor
    }

    contentItem: Item {
        implicitWidth: root.width
        implicitHeight: root.height

        Rectangle {
            width: parent.width * root.visualPosition
            height: parent.height
            radius: height
            color: Theme.buttonPrimaryColor
        }
    }
}
