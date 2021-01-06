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
    property int bottomContentY: 0
    property int previousContentY: 0
    property bool addAnimationEnabled: false
    property bool flickToBottomButtonVisible: false
    property int unreadMessagesCount: 0
    onUnreadMessagesCountChanged: chatList.autoFlickToBottomController()

    ListView {
        id: messagesListView

        property var contextMenu: null

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

        bottomMargin: flick.calculateContentMargin()
        Behavior on bottomMargin {
            NumberAnimation { duration: isReady ? Theme.animationDuration : 0; easing.type: Easing.InOutCubic }
        }

        ScrollBar.vertical: MessageListViewScrollBar {}

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
                chatList.init()
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                forceActiveFocus()
                mouse.accepted = false
            }
            onWheel: {
                if (messagesListView.contextMenu) {
                    messagesListView.contextMenu.visible = false
                }
                wheel.accepted = false
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
            thisDay: day
            isOwnMessage: model.senderId === controllers.users.currentUserId
            statusIcon: isOwnMessage ? model.statusIcon : ""
            attachmentPictureThumbnailWidth: model.attachmentPictureThumbnailSize.width
            attachmentPictureThumbnailHeight: model.attachmentPictureThumbnailSize.height
            messageId: model.id

            onSaveAttachmentAs: function(messageId) {
                saveAttachmentAsDialog.messageId = messageId
                saveAttachmentAsDialog.attachmentType = model.attachmentTypeIsPicture ? AttachmentTypes.picture : AttachmentTypes.file
                saveAttachmentAsDialog.open()
            }

            onOpenContextMenu: function(messageId, mouse, contextMenu) {
                if (!contextMenu.enabled) {
                    return
                }
                messagesListView.contextMenu = contextMenu
                var coord = mapToItem(messagesListView, mouse.x, mouse.y)
                contextMenu.x = coord.x - (Platform.isMobile ? contextMenu.width : 0)
                contextMenu.y = coord.y
                contextMenu.parent = messagesListView
                contextMenu.open()
            }
        }
    }

    QtObject {
        id: chatList

        function init() {
            isReady = true
            flick.setBotomContentY()
        }

        function countChangedController() {

            if (flick.ownMessage()) {
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
                flick.setBotomContentY()
                return
            }

            if (unreadMessagesCount > 0) {
                flick.flickToBottomButtonVisible(true)
                return
            }

            if (!flick.chatAtBottom()) {
                let diff = messagesListItem.bottomContentY - messagesListView.contentY
                if (diff > 100) {
                    flick.flickToBottomButtonVisible(true)
                }
                flick.setNewContentY()
                return
            }

        }

    }

    QtObject {
        id: flick

        function ownMessage() {
            return models.messages.lastMessageSenderId() === controllers.users.currentUserId
        }

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

        function setBotomContentY() {
            messagesListItem.bottomContentY = messagesListView.contentY
        }

        function setChatToBottom() { // Breaks unreadMes... separator logic
            messagesListView.cancelFlick()
            flickToStartAnimation.running = false
            let currentPosition = messagesListView.contentY

            messagesListView.positionViewAtBeginning()
            let destinationPosition = messagesListView.contentY
            flick.setBotomContentY()
            let diff = destinationPosition - currentPosition
            if (diff > 400) {
                currentPosition = destinationPosition - 400
            }

            console.log("setChatToBottom", flick.chatAtBottom())

            flickToStartAnimation.from = currentPosition
            flickToStartAnimation.to = destinationPosition
            flickToStartAnimation.running = true
        }

        function calculateContentMargin() {
            if (messagesListView.contentHeight < messagesListItem.height) {
                return messagesListItem.height - messagesListView.contentHeight
            } else {
                return 0
            }
        }
    }

    Connections {
        target: models.messages
        function onMessageAdding() {
            messagesListItem.addAnimationEnabled = true
        }
    }
}
