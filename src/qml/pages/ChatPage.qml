import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"
import "../components"

Page {
    id: chatPage

    readonly property var appState: app.stateManager.chatState
    readonly property var contactId: controllers.chats.currentContactId
    property real chatListViewHeight: 0

    QtObject {
        id: d
        readonly property real listSpacing: 5
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: Control {
        id: headerControl
        width: parent.width
        height: Theme.headerHeight
        z: 1

        background: Rectangle {
            color: Theme.chatBackgroundColor
            anchors.leftMargin: 5
            anchors.rightMargin: 5

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                height: 1
                color: Theme.chatSeparatorColor
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.smallMargin
            anchors.rightMargin: Theme.smallMargin

            ImageButton {
                image: "Arrow-Left"
                onClicked: app.stateManager.goBack()
            }

            Column {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.smallMargin
                Label {
                    text: contactId
                    font.pointSize: UiHelper.fixFontSz(15)
                    color: Theme.primaryTextColor
                    font.bold: true
                }

                Label {
                    topPadding: 2
                    text: appState.lastActivityText
                    font.pointSize: UiHelper.fixFontSz(12)
                    color: Theme.secondaryTextColor
                }
            }
        }
    }

//  ChatListItem
    Item {
        id: chatListItem
        anchors.fill: parent

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
            onContentYChanged: chatListItem.autoFlickToBottomController()
            Behavior on contentY {
                NumberAnimation {
                    duration: chatListItem.animationEnabled ? Theme.animationDuration : 0
                    easing.type: Easing.InOutCubic
                }
            }

//            onHeightChanged: chatListItem.heightChangedController() not working yet

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
                onCountChanged: chatListItem.flickToBottomController()

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

//      Funtions
        function flickToBottomController(newItem = true) {
            if (chatListItem.autoFlickToBottom) {
                if (chatListItem.animationEnabled) {
                    flickChatToBottom(newItem)
                } else {
                    setChatToBottom()
                }
                return
            }

//            if (newItem) {
//                let lastItem = chatListView.itemAtIndex(chatListView.count - 1)
//                if (lastItem) {
//                }
//                unreadMessages += 1
//            }
        }

        function isChatBottom() { // not using
            if (chatFlickable.atYEnd) {
                chatListItem.animationEnabled = true
            } else {
                chatListItem.animationEnabled = false
                chatFlickable.contentY = chatListItem.chatContentHeight
                chatListItem.animationEnabled = true
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
                    console.log("INSIDE: ", lastItem.height, lastItem.body, lastItem.firstInRow)
                }
            }

            chatFlickable.contentY = chatListItem.chatContentHeight - contentDiff
        }

        function heightChangedController() {
            let val = 0
            if (chatListItem.height < chatListItem.previousHeight) {
                chatListItem.flickToBottomController(false)
                val = 1
            }
            chatListItem.previousHeight = chatListItem.height

            console.log("heightChangedController", val)
        }

        function autoFlickToBottomController() {

            if (flk.chatAtBottom()) {
                flk.setAutoFlick(true, false)
                flk.setNewContentY()
                return
            }

            if (flk.unreadMessages() || flk.scrollingDown()) {
                flk.setAutoFlick(flk.chatAtBottom(), !flk.chatAtBottom())
                flk.setNewContentY()
                return
            }

            if (!flk.scrollingDown()) {
                flk.setAutoFlick(false, false)
                flk.setNewContentY()
                return
            }
        }

        QtObject {
            id: flk

            function scrollingDown() {
                return chatFlickable.contentY > chatFlickable.previousContentY
            }

            function chatAtBottom() {
                let autoFlickMinDiff = chatListItem.height + chatListItem.minHeightToDisableAutoFlick
                return chatFlickable.contentHeight - chatFlickable.contentY < autoFlickMinDiff
            }

            function unreadMessages() {
                return false
            }

            function setNewContentY() {
                chatFlickable.previousContentY = chatFlickable.contentY
            }

            function setAutoFlick(autoFlick, buttonVisible) {
                chatListItem.autoFlickToBottom = autoFlick
                chatListItem.autoFlickToBottomButtonVisible = buttonVisible
            }
        }
    }

//  Components
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
                chatListItem.flickToBottomController(false)
            }
        }

        Rectangle {
            anchors {
                horizontalCenter: scrollToBottomButton.right
                horizontalCenterOffset: -5
                verticalCenter: scrollToBottomButton.top
                verticalCenterOffset: 5
            }
            width: 20
            height: width
            radius: height
            color: Theme.buttonPrimaryColor
        }
    }

    footer: ChatMessageInput {
        id: footerControl
    }

    Item {
        SelectAttachmentsDialog {
            id: saveAttachmentAsDialog
            selectExisting: false

            property string messageId: ""

            onAccepted: controllers.attachments.saveAs(messageId, fileUrl)
        }

        SoundEffect {
            id: messageReceived
            source: "../resources/sounds/message-received.wav"
        }

        SoundEffect {
            id: messageSent
            source: "../resources/sounds/message-sent.wav"
        }
    }

    Connections {
        target: appState

        function onMessageSent() {
            messageSent.play()
        }
    }
}

