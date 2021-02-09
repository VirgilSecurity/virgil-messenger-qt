import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Rectangle {
    id: root

    width: label.width + Theme.margin * 2
    height: 40
    radius: height * 0.5
    color: Theme.chatSeparatorColor
    anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin: height
    }

    property alias text: label.text

    signal clicked()

    Behavior on opacity {
        NumberAnimation { duration: Theme.animationDuration; easing.type: Easing.OutCubic }
    }

    Behavior on width {
        NumberAnimation { duration: Theme.shortAnimationDuration; easing.type: Easing.OutCubic }
    }

    Label {
        id: label
        anchors.centerIn: parent
        elide: Label.ElideRight
        font.family: Theme.mainFont
        font.pointSize: UiHelper.fixFontSz(12)
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        color: Theme.primaryTextColor
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            cursorShape = Qt.PointingHandCursor
            root.scale = 1.02
        }

        onExited:  {
            cursorShape = Qt.ArrowCursor
            root.scale = 1
        }

        onClicked: root.clicked()
    }
}
