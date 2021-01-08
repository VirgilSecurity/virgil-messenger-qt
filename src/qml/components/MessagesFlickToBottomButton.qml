import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12

import "../base"
import "../theme"
import "../components"

Item {
    anchors {
        bottom: parent.bottom
        right: parent.right
        bottomMargin: Theme.margin
        rightMargin: Theme.margin
    }
    width: scrollToBottomButton.width + Theme.margin
    height: scrollToBottomButton.height + Theme.margin
    opacity: messagesListItem.flickToBottomButtonVisible ? 1 : 0
    visible: opacity > 0 ? true : false
    layer.enabled: true

    Behavior on opacity {
        NumberAnimation { duration: Theme.shortAnimationDuration; easing.type: Easing.InOutCubic }
    }

    ImageButton {
        id: scrollToBottomButton
        anchors.centerIn: parent
        imageSize: 34
        backgroundColor: Theme.inputBackgroundColor
        image: "Arrow-Right"
        rotation: 90

        onClicked: {
            flick.setChatToBottom()
        }
    }

    Rectangle {
        anchors {
            horizontalCenter: scrollToBottomButton.right
            horizontalCenterOffset: -5
            verticalCenter: scrollToBottomButton.top
            verticalCenterOffset: 5
        }
        width: messagesListItem.unreadMessagesCount > 9 ? 23: 18
        height: 18
        radius: height
        color: Theme.buttonPrimaryColor
        visible: flick.unreadMessages()

        Label {
            id: unreadMessagesCount
            text: messagesListItem.unreadMessagesCount > 9 ? "9+" : messagesListItem.unreadMessagesCount
            color: Theme.primaryTextColor
            font.pixelSize: UiHelper.fixFontSz(12)
            anchors.centerIn: parent
        }
    }
}
