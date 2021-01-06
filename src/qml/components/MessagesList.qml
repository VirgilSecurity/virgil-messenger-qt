import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12

import "../base"
import "../theme"
import "../components"

Item {
    id: messagesListItem

    property bool isReady: false
    property int previousContentY: 0
    property int previousCount: 0
    property bool flickToBottomButtonVisible: false
    property int unreadMessagesCount: 0
    onUnreadMessagesCountChanged: chatList.autoFlickToBottomController()

    ListView {
        id: messagesListView
        anchors {
            fill: parent
            leftMargin: Theme.margin
            rightMargin: Theme.margin
        }

        verticalLayoutDirection: ListView.BottomToTop
        spacing: d.listSpacing
        model: models.messages.proxy
        footer: Item {
            width: messagesListView.width
            height: Theme.margin
        }

        delegate: messageDelegate
        interactive: true

        onCountChanged: chatList.countChangedController()
        onContentYChanged: chatList.autoFlickToBottomController()

        displaced: Theme.displacedTransition

        ScrollBar.vertical: ScrollBar {
            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
                rightMargin: -(Theme.margin - 2)
            }
        }

        NumberAnimation {
            id: flickToStartAnimation
            target: messagesListView
            property: 'contentY'
            duration: Theme.shortAnimationDuration
            easing.type: Easing.InOutCubic
            alwaysRunToEnd: true
        }

        Timer { // init
            running: true
            repeat: false
            interval: 10
            onTriggered: {
                isReady = true
            }
        }
    }

    MessagesFlickToBottomButton {}

    // OTHER

    Component {
        id: messageDelegate

        ChatMessage {
            readonly property real fullWidth: messagesListView.width - leftIndent
            width: messagesListView.width
            maxWidth: Platform.isMobile ? (fullWidth - 2 * Theme.margin) : fullWidth

            thisIndex: index
//            thisDay: day
//            isOwnMessage: model.senderId === controllers.users.currentUserId
//            statusIcon: isOwnMessage ? model.statusIcon : ""
//            attachmentPictureThumbnailWidth: model.attachmentPictureThumbnailSize.width
//            attachmentPictureThumbnailHeight: model.attachmentPictureThumbnailSize.height
//            messageId: model.id

            isOwnMessage: model.isOwnMessage
            statusIcon: model.statusIcon
            attachmentPictureThumbnailWidth: 0
            attachmentPictureThumbnailHeight: 0
            messageId: model.messageId

            onSaveAttachmentAs: function(messageId) {
                saveAttachmentAsDialog.messageId = messageId
                saveAttachmentAsDialog.attachmentType = model.attachmentTypeIsPicture ? AttachmentTypes.picture : AttachmentTypes.file
                saveAttachmentAsDialog.open()
            }

            onOpenContextMenu: function(messageId, mouse, contextMenu) {
                if (!contextMenu.enabled) {
                    return
                }
                chatListView.contextMenu = contextMenu
                var coord = mapToItem(chatListView, mouse.x, mouse.y)
                contextMenu.x = coord.x - (Platform.isMobile ? contextMenu.width : 0)
                contextMenu.y = coord.y
                contextMenu.parent = chatListView
                contextMenu.open()
            }
        }
    }

    QtObject {
        id: chatList

        function countChangedController() {

            if (tempModel.get(0).isOwnMessage) {
                flick.setChatToBottom()
                return
            }

            if (!flick.chatAtBottom()) {
                unreadMessagesCount += 1
            }
        }

        function autoFlickToBottomController() {

            if (flick.chatAtBottom()) {
                flick.flickToBottomButtonVisible(false)
                flick.readAllMessages()
                return
            }

            if (unreadMessagesCount > 0) {
                flick.flickToBottomButtonVisible(true)
                return
            }

            if (flick.scrollingDown()) {
                flick.flickToBottomButtonVisible(true)
                flick.setNewContentY()
                return
            }

            if (!flick.scrollingDown()) {
                flick.flickToBottomButtonVisible(false)
                flick.setNewContentY()
                return
            }

        }

    }

    QtObject {
        id: flick

        function chatAtBottom() {
            return messagesListView.atYEnd
        }

        function scrollingDown() {
            return messagesListView.contentY > messagesListItem.previousContentY
        }

        function unreadMessages() {
            return messagesListItem.unreadMessagesCount > 0
        }

        function readAllMessages() {
            messagesListItem.unreadMessagesCount = 0
        }

        function flickToBottomButtonVisible(value) {
            messagesListItem.flickToBottomButtonVisible = value
        }

        function setNewContentY() {
            messagesListItem.previousContentY = messagesListView.contentY
        }

        function setChatToBottom() {
            messagesListView.cancelFlick()
            flickToStartAnimation.running = false
            let currentPosition = messagesListView.contentY

            messagesListView.positionViewAtBeginning()
            let destinationPosition = messagesListView.contentY

            let diff = destinationPosition - currentPosition
            if (diff > 400) {
                currentPosition = destinationPosition - 400
            }

            console.log("setChatToBottom", flick.chatAtBottom())

            flickToStartAnimation.from = currentPosition
            flickToStartAnimation.to = destinationPosition
            flickToStartAnimation.running = true
        }
    }
}
