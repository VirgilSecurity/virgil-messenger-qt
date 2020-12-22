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
        anchors.fill: parent
        contentHeight: chatListItem.chatContentHeight

        property real previousContentY: contentY
        onContentYChanged: chatList.autoFlickToBottomController()
        Behavior on contentY {
            NumberAnimation {
                duration: chatListItem.animationEnabled ? Theme.animationDuration : 0
                easing.type: Easing.InOutCubic
            }
        }

//            onHeightChanged: chatList.heightChangedController() not working yet

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
        interval: Theme.animationDuration
        onTriggered: {
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

            let contentDiff = chatListItem.height
            if (newItem === true) {
                let lastItem = chatListView.itemAtIndex(chatListView.count - 1)
                if (lastItem) {
                    contentDiff -= (lastItem.height + chatListView.spacing)
                }
            }

            chatFlickable.contentY = chatListItem.chatContentHeight - contentDiff
        }

        function autoFlickToBottomController() {

            if (flick.chatAtBottom()) {
                flick.setAutoFlick(true, false)
                flick.setNewContentY()
                flick.readAllMessages()
                return
            }

            if (flick.unreadMessages() || flick.scrollingDown()) {
                flick.setAutoFlick(flick.chatAtBottom(), !flick.chatAtBottom())
                flick.setNewContentY()
                return
            }

            if (!flick.scrollingDown()) {
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
                    if (flick.chatAtBottom()) {
                        flick.scrollDown()
                    } else {
                        chatListItem.unreadMessages += 1
                        flickToBottomController(true)
                        autoFlickToBottomController()
                    }
                }
            } else {
                console.log("This is big error. Trying to access not created yet item")
            }
        }

        function heightChangedController() {
            let val = 0
            if (chatListItem.height < chatListItem.previousHeight) {
                chatList.flickToBottomController(false)
                val = 1
            }
            chatListItem.previousHeight = chatListItem.height

            console.log("heightChangedController", val)
        }
    }

    QtObject {
        id: flick

        function scrollingDown() {
            return chatFlickable.contentY > chatFlickable.previousContentY
        }

        function chatAtBottom() {
            let autoFlickMinDiff = chatListItem.height + chatListItem.minHeightToDisableAutoFlick
            return chatFlickable.contentHeight - chatFlickable.contentY < autoFlickMinDiff
        }

        function unreadMessages() {
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
    }

// Components
    Item {
        anchors {
            bottom: parent.bottom
            right: parent.right
            bottomMargin: Theme.margin * 3
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
            backgroundColor: Theme.chatSeparatorColor
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
            width: chatListItem.unreadMessages > 9 ? 25 : 20
            height: 20
            radius: height
            color: Theme.buttonPrimaryColor
            visible: flick.unreadMessages()

            Label {
                id: unreadMessagesCount
                text: chatListItem.unreadMessages > 9 ? "9+" : chatListItem.unreadMessages
                color: Theme.primaryTextColor
                font.pixelSize: UiHelper.fixFontSz(14)
                anchors.centerIn: parent
            }
        }
    }
}
