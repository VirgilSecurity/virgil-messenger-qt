import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Flickable {
    id: flowListView
    contentHeight: flow.childrenRect.height

    property alias delegate: repeater.delegate
    property alias model: repeater.model
    property alias spacing: flow.spacing
    readonly property alias count: repeater.count

    Flow {
        id: flow
        anchors.fill: parent
        spacing: 0.5 * Theme.smallSpacing
        add: Theme.addTransition
        move: Theme.moveTransition

        Repeater {
            id: repeater
        }
    }
}
