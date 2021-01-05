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
    property bool messageAddAnimationEnabled: false
    property bool flickToBottomButtonVisible: false
    property int unreadMessagesCount: 0
    property int previousContentY: 0

    ListView {
        id: messagesListView
        anchors {
            fill: parent
            leftMargin: Theme.margin
            rightMargin: Theme.margin
        }

        verticalLayoutDirection: ListView.BottomToTop
        spacing: d.listSpacing
        model: tempModel
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
                messageAddAnimationEnabled = true
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

            if (messagesListView.count > 0 && isReady) {
                messageAddAnimationEnabled = true
            }

            if (tempModel.get(0).isOwnMessage) {
                flick.setChatToBottom()
                return
            }

            if (!flick.chatAtBottom()) {
//                console.log(">>>>>>> unreadMessagesCount += 1")
                unreadMessagesCount += 1
            }
        }

        function autoFlickToBottomController() {

            if (flick.chatAtBottom()) {
                flick.flickToBottomButtonVisible(false)
                flick.readAllMessages()
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



    // TEMP
    ListModel {
        id: tempModel

        Component.onCompleted: {
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
            tempModel.insert(0, getRandomDict())
        }
    }

    Timer {
        running: false
        repeat: true
        interval: 3000
        onTriggered: {
            tempModel.insert(0, getRandomDict())
        }
    }

    function getRandomDict() {
        let listOfProperties = {
            "body": getRandom(possibleMessages),
            "displayTime": Date.now(),
            "senderUsername": "Test Guy",
            "isOwnMessage": (Math.random() < 0.5),
            "statusIcon": '',
            "messageId": Math.random().toString(36).substring(7),
            "inRow": (Math.random() < 0.5),
            "firstInRow": (Math.random() < 0.5),
            "isBroken": false,

            "attachmentId": '',
            "attachmentTypeIsFile": false,
            "attachmentTypeIsPicture": false,
            "attachmentIsLoading": false,
            "attachmentIsLoaded": false,
            "attachmentIconPath": "",
            "attachmentPictureThumbnailWidth": 0,
            "attachmentPictureThumbnailHeight": 0,
            "attachmentDisplaySize": 0,
            "attachmentDisplayText": "",
            "attachmentDisplayProgress": 0,
            "attachmentBytesTotal": 0,
            "attachmentBytesLoaded": 0,
            "attachmentFileExists": false
        }

        if (listOfProperties["isOwnMessage"]) {
            console.log(">>>>>>>> OWN MESSAGE", Math.random() * 100)
        }

        return listOfProperties
    }

    function getRandom(list) {
        return list[Math.floor((Math.random()*list.length))]
    }

    property var possibleMessages: [
        'Hello my friend, this is short message',
        'Hello my friend, this is long message. I need to see how that works, so I added more letters',
        'To access and use all the features of Apple Card, you must add Apple Card to Wallet on an iPhone or iPad with iOS 12.4 or later or iPadOS. To manage Apple Card Monthly Installments, you need an iPhone with iOS 13.2 or later or an iPad with iPadOS 13.2 or later. Update to the latest version of iOS or iPadOS by going to Settings > General > Software Update. Tap Download and Install.',
        'Available for qualifying applicants in the United States.',
        '5G goes Pro. A14 Bionic rockets past every other smartphone chip. The Pro camera system takes low-light photography to the next level — with an even bigger jump on iPhone 12 Pro Max. And Ceramic Shield delivers four times better drop performance. Let’s see what this thing can do.',
        'Ok'
    ]

}
