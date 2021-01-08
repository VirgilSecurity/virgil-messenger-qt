import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../base"
import "../theme"

Item {
    id: newMessageItem
    property bool isVisible: index === (unreadMessagesCount - 1)
    onIsVisibleChanged: {
        if (isVisible) {
            newMessageItem.opacity = 1
        } else {
            opacityOffDelay.restart()
        }
    }

    opacity: 0
    visible: newMessageItem.opacity > 0

    Timer {
        id: opacityOffDelay
        running: false
        repeat: false
        interval: 1500
        onTriggered: newMessageItem.opacity = 0
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }

    Item {
        width: parent.width
        height: unreadMessageRec.height
        anchors.verticalCenter: parent.verticalCenter
        layer.enabled: true

        Rectangle {
            width: parent.width
            height: 1
            color: Theme.buttonPrimaryColor
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            id: unreadMessageRec
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            height: newMessage.height + stdSmallMargin
            width: newMessage.width + stdSmallMargin
            radius: 3
            color: Theme.buttonPrimaryColor

            Rectangle {
                anchors {
                    horizontalCenter: parent.left
                    horizontalCenterOffset: 2
                    verticalCenter: parent.verticalCenter
                }

                height: parent.height * 0.65
                width: height
                rotation: 45
                color: parent.color
            }

            Label {
                id: newMessage
                text: qsTr("NEW")
                color: Theme.primaryTextColor
                font.pixelSize: UiHelper.fixFontSz(8)
                font.bold: true
                anchors.centerIn: parent
            }
        }
    }
}
