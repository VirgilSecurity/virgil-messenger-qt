import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12

import "../base"
import "../theme"
import "../components"

Item {
    id: chatListItem

    property real previousHeight: chatListItem.height
    property real chatContentHeight: chatListView.height
    property bool smartChatEnabled: false
    property bool autoFlickToBottom: true
    property bool autoFlickToBottomButtonVisible: false
    readonly property int minHeightToDisableAutoFlick: 100

    property int unreadMessages: 0
    property string idOfFirstUnreadMessage: ""

    property bool isInsert: false

    Flickable {
        id: chatFlickable

        property real previousContentY: contentY
        property real previousContentHeight: contentHeight

        anchors.fill: parent
        contentHeight: chatListItem.chatContentHeight
      //onContentHeightChanged: chatList.contentHeightChangedController() // breaks onCountChanged TODO(dz)
        onHeightChanged: chatList.heightChangedController() // when changing height - new messages are not on bottom TODO(dz)
        onContentYChanged: chatList.autoFlickToBottomController()
        Behavior on contentY {
            NumberAnimation {
                duration: chatListItem.smartChatEnabled ? Theme.animationDuration : 0
                easing.type: Easing.InOutCubic
                alwaysRunToEnd: true
            }
        }

        MessageListView {
            id: chatListView

            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.margin
                rightMargin: Theme.margin
                top: parent.top
            }

            interactive: false
            height: contentHeight
            onCountChanged: chatList.countChangedController()

            footer: Item {
                width: parent.width
                height: Theme.smallSpacing
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }

    Timer {
        running: true
        repeat: false
        interval: 1
        onTriggered: {
            chatListItem.autoFlickToBottom = true
            chatList.flickToBottomController(false)
            chatListItem.smartChatEnabled = true
        }
    }

// Funсtions

    QtObject {
        id: chatList

        function countChangedController() {
            if (chatListItem.smartChatEnabled) {
                newMessageController()
            }
        }

        function flickToBottomController(newItem = true) {
            if (chatListItem.autoFlickToBottom) {
                if (chatListItem.smartChatEnabled) {
                    flickChatToBottom(newItem)
                } else {
                    setChatToBottom()
                }
            }
        }

        function setChatToBottom() {
            if (chatListItem.chatContentHeight < chatListItem.height) {
                chatFlickable.contentY = 0
                return
            }

            chatFlickable.contentY = chatListItem.chatContentHeight - chatListItem.height
        }

        function flickChatToBottom(newItem) {
            if (chatListItem.chatContentHeight < chatListItem.height) {
                chatFlickable.contentY = 0
                return
            }

            if (newItem === true && chatListView.count > 1) {
                chatFlickable.contentY = chatListItem.chatContentHeight - flick.calculateContentDiff(chatListItem.height)
            } else if (newItem === false) {
                chatFlickable.contentY = chatListItem.chatContentHeight - chatListItem.height
            }
        }

        function autoFlickToBottomController() {

            if (flick.isChatAtBottom()) {
                flick.setAutoFlick(true, false)
                flick.setNewContentY()
                flick.readAllMessages()
                return
            }

            if (flick.hasUnreadMessages() || flick.isScrollingDown()) {
                flick.setAutoFlick(flick.isChatAtBottom(), !flick.isChatAtBottom())
                flick.setNewContentY()
                return
            }

            if (!flick.isScrollingDown()) {
                flick.setAutoFlick(false, false)
                flick.setNewContentY()
                return
            }
        }

        function newMessageController() {
            let lastItem = chatListView.itemAtIndex(chatListView.count - 1)
            if (lastItem) {
                if (lastItem.isOwnMessage) {
                    flick.scrollDown()
                } else {
                    if (flick.isChatAtBottom()) {
                        flick.scrollDown()
                    } else {
                        if (chatListItem.unreadMessages === 0) {
                            idOfFirstUnreadMessage = lastItem.messageId
                        }

                        chatListItem.unreadMessages += 1
                        flickToBottomController(true)
                        autoFlickToBottomController()
                    }
                }
            }
        }

        function heightChangedController() {
            if (!chatListItem.smartChatEnabled) {
                return
            }

            if (chatListItem.height < chatListItem.previousHeight) {
                chatListItem.smartChatEnabled = false
                chatFlickable.contentY += chatListItem.previousHeight - chatListItem.height
                chatListItem.smartChatEnabled = true
            }

            if (chatListItem.height > chatListItem.previousHeight) {
                chatListItem.smartChatEnabled = false
                chatFlickable.contentY -= chatListItem.height - chatListItem.previousHeight
                chatListItem.smartChatEnabled = true
            }

            chatListItem.previousHeight = chatListItem.height
        }

        function contentHeightChangedController() {
            if (chatListItem.smartChatEnabled) {
                if (!chatListItem.isInsert) {
                    chatListItem.smartChatEnabled = false
                    let contentYPercent = chatFlickable.contentY / chatFlickable.previousContentHeight
                    chatFlickable.contentY = chatFlickable.contentHeight * contentYPercent
                    chatListItem.smartChatEnabled = true
                }
            }

            chatFlickable.previousContentHeight = chatFlickable.contentHeight
        }
    }

    QtObject {
        id: flick

        function isScrollingDown() {
            return chatFlickable.contentY > chatFlickable.previousContentY
        }

        function isChatAtBottom() {
            let autoFlickMinDiff = chatListItem.height + chatListItem.minHeightToDisableAutoFlick
            return chatFlickable.contentHeight - chatFlickable.contentY < autoFlickMinDiff
        }

        function hasUnreadMessages() {
            return chatListItem.unreadMessages > 0
        }

        function readAllMessages() {
            chatListItem.unreadMessages = 0
            idOfFirstUnreadMessage = ""
        }

        function setNewContentY() {
            chatFlickable.previousContentY = chatFlickable.contentY
        }

        function setAutoFlick(autoFlick, buttonVisible) {
            chatListItem.autoFlickToBottom = autoFlick
            chatListItem.autoFlickToBottomButtonVisible = buttonVisible
        }

        function scrollDown() {
            // use it only if you don't need to perform checks
            chatListItem.autoFlickToBottom = true
            chatList.flickToBottomController(true)
        }

        function calculateContentDiff(contentDiff) {
            let lastItem = chatListView.itemAtIndex(chatListView.count - 1)
            let prevItem = chatListView.itemAtIndex(chatListView.count - 2)

            if (lastItem && prevItem) {

                if (lastItem.thisDay.valueOf() === prevItem.thisDay.valueOf()) {

                    if (lastItem.firstInRow) {
                        return contentDiff - (lastItem.height + chatListView.spacing + Theme.smallSpacing)
                    } else {
                        return contentDiff - (lastItem.height + chatListView.spacing)
                    }

                } else {
                    return contentDiff - (lastItem.height + chatListView.separatorHeight + Theme.smallSpacing)
                }

            } else {
                console.log("There was an error. ChatListMessage was not created yet!")
                return 0
            }
        }
    }

// Components

    Item {
        anchors {
            bottom: parent.bottom
            right: parent.right
            bottomMargin: Theme.margin
            rightMargin: Theme.margin
        }
        width: scrollToBottomButton.width + Theme.margin
        height: scrollToBottomButton.height + Theme.margin
        opacity: chatListItem.autoFlickToBottomButtonVisible ? 1 : 0
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
                chatListItem.autoFlickToBottom = true
                chatList.flickToBottomController(false)
            }
        }

        Rectangle {
            anchors {
                horizontalCenter: scrollToBottomButton.right
                horizontalCenterOffset: -5
                verticalCenter: scrollToBottomButton.top
                verticalCenterOffset: 5
            }
            width: chatListItem.unreadMessages > 9 ? 23: 18
            height: 18
            radius: height
            color: Theme.buttonPrimaryColor
            visible: flick.hasUnreadMessages()

            Label {
                id: unreadMessagesCount
                text: chatListItem.unreadMessages > 9 ? "9+" : chatListItem.unreadMessages
                color: Theme.primaryTextColor
                font.pixelSize: UiHelper.fixFontSz(12)
                anchors.centerIn: parent
            }
        }
    }
}
