import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QuickFuture 1.0
import QtQuick.Window 2.12
import QtMultimedia 5.12
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"
import "../components"

Page {
    id: chatPage

    property string recipient

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: Control {
        id: headerControl
        width: parent.width
        height: 60
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
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            ImageButton {
                image: "Arrow-Left"
                onClicked: mainView.back()
            }

            Column {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Label {
                    text: ConversationsModel.recipient
                    font.pointSize: UiHelper.fixFontSz(15)
                    color: Theme.primaryTextColor
                    font.bold: true
                }

                Label {
                    id: lastActivityLabel
                    topPadding: 2
                    text: " "
                    font.pointSize: UiHelper.fixFontSz(12)
                    color: Theme.secondaryTextColor
                }
            }
        }
    }


    ListView {
        id: listView

        property real previousHeight: 0.0
        property var contextMenu: null

        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        section.property: "day"
        section.delegate: ChatDateSeporator {
            date: section
        }

        spacing: 5
        delegate: ChatMessage {
            maxWidth: Math.min(root.width - 220, 800)

            body: model.message
            time: Qt.formatDateTime(model.timestamp, "hh:mm")
            nickname: model.author
            isUser: model.author === Messenger.currentUser
            status: isUser ? model.status : "none"
            failed: (model.status == 4) && (!attachmentId || attachmentStatus == Enums.AttachmentStatus.Failed)
            messageId: model.messageId

            inRow: model.messageInARow
            firstInRow: model.firstMessageInARow

            attachmentId: model.attachmentId
            attachmentBytesTotal: model.attachmentBytesTotal
            attachmentDisplaySize: model.attachmentDisplaySize
            attachmentType: model.attachmentType
            attachmentFilePath: model.attachmentFilePath
            attachmentThumbnailPath: model.attachmentThumbnailPath
            attachmentThumbnailWidth: model.attachmentThumbnailWidth
            attachmentThumbnailHeight: model.attachmentThumbnailHeight
            attachmentBytesLoaded: model.attachmentBytesLoaded
            attachmentStatus: model.attachmentStatus
            attachmentDownloaded: model.attachmentDownloaded

            onSaveAttachmentAs: function(messageId) {
                saveAttachmentAsDialog.messageId = messageId
                saveAttachmentAsDialog.attachmentType = attachmentType
                saveAttachmentAsDialog.open()
            }

            onDownloadOpenAttachment: function(messageId, isPicture) {
                (isPicture ? Messenger.openAttachment : Messenger.downloadAttachment)(messageId)
            }

            onOpenContextMenu: function(messageId, mouse, contextMenu) {
                listView.contextMenu = contextMenu
                var coord = mapToItem(listView, mouse.x, mouse.y)
                contextMenu.x = coord.x - (Platform.isMobile ? contextMenu.width : 0)
                contextMenu.y = coord.y
                contextMenu.parent = listView
                contextMenu.open()
            }
        }

        ScrollBar.vertical: ScrollBar { }

        Component.onCompleted: {
            countChanged.connect(positionViewAtEnd)
            heightChanged.connect(function() {
                if (height < previousHeight) {
                    positionViewAtEnd()
                }
                previousHeight = height
            })
            positionViewAtEnd()
            if (Platform.isIos) {
                positionViewAtEnd() // HACK(fpohtmeh): fix initial positioning in IOS
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                forceActiveFocus()
                mouse.accepted = false
            }
            onWheel: {
                if (listView.contextMenu) {
                    listView.contextMenu.visible = false
                }
                wheel.accepted = false
            }
        }
    }

    footer: ChatMessageInput {
        id: footerControl
        onMessageSending: {
            var future = Messenger.sendMessage(ConversationsModel.recipient, message, attachmentUrl, attachmentType)
            Future.onFinished(future, function(value) {
                messageSent.play()
            })
        }
    }

    SelectAttachmentsDialog {
        id: saveAttachmentAsDialog
        selectExisting: false

        property string messageId: ""

        onAccepted: Messenger.saveAttachmentAs(messageId, fileUrl)
    }

    // Component events

    Component.onCompleted: {
        // configure conversation model to chat with
        // recipient provided as a parameter to this page.

        ConversationsModel.recipient = recipient
        listView.model = ConversationsModel
        ConversationsModel.setAsRead(recipient)
        ChatModel.updateUnreadMessageCount(recipient)

        Messenger.openPreviewRequested.connect(openPreview)
    }

    Component.onDestruction: {
        Messenger.openPreviewRequested.disconnect(openPreview)
    }

    Connections {
        target: Messenger
        function onLastActivityTextChanged(text) {
            lastActivityLabel.text = text
        }
    }

    // Sounds

    SoundEffect {
        id: messageReceived
        source: "../resources/sounds/message-received.wav"
    }

    SoundEffect {
        id: messageSent
        source: "../resources/sounds/message-sent.wav"
    }
}

