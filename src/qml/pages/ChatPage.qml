import QtQuick 2.15
import QtQuick.Controls 2.15
import QtMultimedia 5.15

import "../base"
import "../theme"
import "../components"
import "../components/Dialogs"

Page {
    id: root

    readonly property var appState: app.stateManager.chatState
    property alias showBackButton: pageHeader.showBackButton

    signal infoRequested()

    QtObject {
        id: d
        readonly property var chat: controllers.chats.current
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        id: pageHeader
        title: d.chat.title
        description: (!d.chat.isGroup && !d.chat.lastActivityText) ? " " : d.chat.lastActivityText
        showSeparator: !groupInvitationDialog.visible
        titleClickable: true

        contextMenuVisible: !groupInvitationDialog.visible
        contextMenu: ContextMenu {
            ContextMenuItem {
                text: d.chat.isGroup ? qsTr("Group info") : qsTr("Chat info")
                onTriggered: root.infoRequested()
            }
        }

        onTitleClicked: root.infoRequested()
    }

    footer: ChatMessageInput {
        id: messageInput
        visible: !groupInvitationDialog.visible
    }

    MessagesList {
        anchors.fill: parent
        clip: true
    }

    Item {
        anchors.fill: parent

        GroupInvitationDialog {
            id: groupInvitationDialog
        }

        SelectAttachmentsDialog {
            id: saveAttachmentAsDialog
            selectExisting: false

            property string messageId: ""

            onAccepted: controllers.attachments.saveAs(messageId, fileUrl)
        }

        SoundEffect {
            id: messageReceivedAudio
            source: "../resources/sounds/message-received.wav"
        }

        SoundEffect {
            id: messageSentAudio
            source: "../resources/sounds/message-sent.wav"
        }
    }

    Timer {
        interval: 10

        function runSetFocus() {
            if (root) {
                running = Platform.isDesktop && root.visible
            }
        }

        Component.onCompleted: {
            triggered.connect(messageInput.setFocus)
            root.visibleChanged.connect(runSetFocus);
        }
    }

    Component.onCompleted: appState.messageSent.connect(messageSentAudio.play)

    Connections {
        target: models.messages
        function onMessagesReset() { messageInput.clear() }
    }

    function navigateBack(transition) {
        controllers.chats.closeChat()
        return true
    }
}

