import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

ScrollBar {
    id: root

    anchors {
        right: parent.right
        rightMargin: -Theme.margin
    }

    contentItem: Item {
        implicitWidth: 6
        implicitHeight: 30

        Rectangle {
            height: parent.height
            width: parent.width
            radius: 0.5 * width
            y: parent.height - height
            color: root.pressed ? Theme.mainBackgroundColor : "#59717D"
            opacity: root.active ? 1 : 0

            Behavior on height {
                enabled: !messagesListView.atYBeginning && !messagesListView.atYEnd
                NumberAnimation { duration: Theme.animationDuration }
            }

            Behavior on opacity {
                NumberAnimation { duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
            }
        }
    }
}
