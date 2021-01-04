import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12

import "../base"
import "../theme"
import "../components"

ModelListView {
    id: chatListView

    property var contextMenu: null
    property real separatorHeight: 0

    spacing: d.listSpacing
    section.property: "day"
    section.delegate: separatorDelegate

    model: models.messages
    delegate: messageDelegate

    MouseArea {
        anchors.fill: parent
        onPressed: {
            forceActiveFocus()
            mouse.accepted = false
        }
        onWheel: {
            if (chatListView.contextMenu) {
                chatListView.contextMenu.visible = false
            }
            wheel.accepted = false
        }
    }

    Component {
        id: separatorDelegate

        ChatDateSeporator {
            date: section
            Component.onCompleted: separatorHeight = height
        }
    }

    Component {
        id: messageDelegate

        ChatMessage {
            readonly property real fullWidth: chatListView.width - leftIndent
            width: chatListView.width
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

                app.stateManager.chatState.currentMessageId = messageId

                chatListView.contextMenu = contextMenu
                var coord = mapToItem(chatListView, mouse.x, mouse.y)
                contextMenu.x = coord.x - (Platform.isMobile ? contextMenu.width : 0)
                contextMenu.y = coord.y
                contextMenu.parent = chatListView
                contextMenu.open()
            }
        }
    }
}
