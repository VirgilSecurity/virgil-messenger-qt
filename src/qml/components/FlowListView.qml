import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Flickable {
    id: flowListView
    contentWidth: parent.width
    contentHeight: flow.childrenRect.height

    property alias delegate: repeater.delegate
    property alias model: repeater.model
    property alias count: repeater.count
    property alias spacing: flow.spacing

    Flow {
        id: flow
        anchors.fill: parent

        add: Transition {
            NumberAnimation { property: "scale"; from: 0.9; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
        }

        move: Transition {
            NumberAnimation { properties: "x,y"; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
        }

        Repeater {
            id: repeater
        }
    }
}
