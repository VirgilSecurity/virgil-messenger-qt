import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12
import com.virgilsecurity.messenger 1.0

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
            width: parent.width
            maxWidth: Platform.isMobile ? (fullWidth - 2 * Theme.margin) : fullWidth

            thisIndex: index
            thisDay: day

            body: model.body
            displayTime: model.displayTime
            nickname: model.authorId
            isOwnMessage: model.authorId === controllers.users.userId
            status: isOwnMessage ? model.status : "none"
            messageId: model.id
            inRow: model.inRow
            firstInRow: model.firstInRow
            isBroken: model.isBroken

            attachmentId: model.attachmentId
            attachmentType: model.attachmentType
            attachmentStatus: model.attachmentStatus
            attachmentDisplaySize: model.attachmentDisplaySize
            attachmentDisplayText: model.attachmentDisplayText
            attachmentDisplayProgress: model.attachmentDisplayProgress
            attachmentBytesTotal: model.attachmentBytesTotal
            attachmentBytesLoaded: model.attachmentBytesLoaded
            attachmentImagePath: model.attachmentImagePath
            attachmentThumbnailWidth: model.attachmentImageSize.width
            attachmentThumbnailHeight: model.attachmentImageSize.height
            attachmentFileExists: model.attachmentFileExists

            onSaveAttachmentAs: function(messageId) {
                saveAttachmentAsDialog.messageId = messageId
                saveAttachmentAsDialog.attachmentType = attachmentType
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
}
