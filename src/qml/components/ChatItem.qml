import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"
import "../components"

Item {
    id: chatListItem

    property real previousHeight: 0.0
    property real chatContentHeight: chatListView.height
    property bool animationEnabled: false
    property bool autoFlickToBottom: true
    property bool autoFlickToBottomButtonVisible: false
    readonly property int minHeightToDisableAutoFlick: 100

    property int unreadMessages: 0

    Flickable {
        id: chatFlickable

        property real previousContentY: contentY

        anchors.fill: parent
        contentHeight: chatListItem.chatContentHeight
        onHeightChanged: chatList.heightChangedController()
        onContentYChanged: chatList.autoFlickToBottomController()
        Behavior on contentY {
            NumberAnimation {
                duration: chatListItem.animationEnabled ? Theme.animationDuration : 0
                easing.type: Easing.InOutCubic
            }
        }

        ChatPageList {
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
            chatList.flickChatToBottom(false)

            chatListItem.animationEnabled = true
        }
    }

// Funсtions

    QtObject {
        id: chatList

        function countChangedController() {
            newMessageController()
        }

        function flickToBottomController(newItem = true) {
            if (chatListItem.autoFlickToBottom) {
                if (chatListItem.animationEnabled) {
                    flickChatToBottom(newItem)
                } else {
                    setChatToBottom()
                }
            }
        }

        function setChatToBottom() {
            chatFlickable.contentY = chatListItem.chatContentHeight
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
                    console.log(">>>>>>>>>>>>>>>>> IS OWN MESSAGE")
                } else {
                    if (flick.isChatAtBottom()) {
                        flick.scrollDown()
                        console.log(">>>>>>>>>>>>>>>>> CHAT AT BOTTOM")
                    } else {
                        chatListItem.unreadMessages += 1
                        flickToBottomController(true)
                        autoFlickToBottomController()
                        console.log(">>>>>>>>>>>>>>>>> CHAT NOT AT BOTTOM")
                    }
                }
            }
        }

        function heightChangedController() {
            if (chatListItem.height < chatListItem.previousHeight) {
                chatList.flickToBottomController(false)
            }
            chatListItem.previousHeight = chatListItem.height
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
