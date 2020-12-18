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

            HorizontalRule {
                anchors.bottom: parent.bottom
                anchors.leftMargin: 20
                anchors.rightMargin: 20
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

    ModelListView {
        id: chatListView

        property real previousHeight: 0.0
        property var contextMenu: null
        property bool animationEnabled: false

        anchors {
            fill: parent
            leftMargin: Theme.margin
            rightMargin: Theme.margin
        }

        spacing: d.listSpacing
        section.property: "day"
        section.delegate: ChatDateSeporator {
            date: section
        }

        model: models.messages
        delegate: messageDelegate

        ScrollBar.vertical: ScrollBar { }

        Component.onCompleted: {
            previousHeight = height
            chatListView.positionViewAtEnd()
        }

        onCountChanged: {
            if (chatListView.animationEnabled) {
                flickTimer.restart()
            } else {
                chatListView.positionViewAtEnd()
                animationEnableTimer.restart()
            }
        }

        onHeightChanged: {
            if (height < previousHeight) {
                flickTimer.restart()
            }
            previousHeight = height
        }

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

        Timer {
            id: animationEnableTimer
            repeat: false
            interval: Theme.animationDuration
            onTriggered: {
                chatListView.animationEnabled = true
            }
        }

        Timer {
            id: flickTimer
            repeat: false
            interval: 50
            onTriggered: {
                chatListView.flickLastMessage()
                repeatflickTimer.restart()
            }
        }

        Timer {
            id: repeatflickTimer
            repeat: false
            interval: Theme.animationDuration
            onTriggered: {
                chatListView.flickLastMessage()
            }
        }

        function flickLastMessage() {
            let position = chatListView.contentY
            chatListView.positionViewAtEnd()
            let destPosition = chatListView.contentY

            flickAnimation.from = position
            flickAnimation.to = destPosition
            flickAnimation.start()
        }

        NumberAnimation {
            id: flickAnimation
            target: chatListView
            property: "contentY"
            duration: Theme.animationDuration
        }
    }

    Component {
        id: messageDelegate

        ChatMessage {
            readonly property real fullWidth: root.width - 2 * Theme.margin - leftIndent

            maxWidth: Platform.isMobile ? (fullWidth - 2 * Theme.margin) : fullWidth

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
