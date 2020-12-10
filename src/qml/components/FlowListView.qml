import QtQuick 2.12
import QtQuick.Controls 2.12

//import "../base"
//import "../components"
import "../theme"

Flickable {
    id: flowListView
    contentWidth: parent.width
    contentHeight: headerHeight + flow.childrenRect.height + footerHeight

    property alias count: repeater.count
    property int currentIndex: -1
    property variant currentItem
    property alias delegate: repeater.delegate
    property alias flow: flow.flow
    property alias model: repeater.model
    property alias spacing: flow.spacing
    property alias headerHeight: flow.anchors.topMargin
    property alias footerHeight: flow.anchors.bottomMargin

    onCurrentIndexChanged: currentItem = model.get(currentIndex)

    Flow {
        id: flow
        anchors {
            fill: parent
            topMargin: 5
            bottomMargin: 5
        }

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
