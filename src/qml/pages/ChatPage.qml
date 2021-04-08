import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtMultimedia 5.15

import "../base"
import "../theme"
import "../components"
import "../components/Dialogs"

Page {
    id: chatPage

    readonly property var appState: app.stateManager.chatState
    property real chatListViewHeight: 0

    QtObject {
        id: d
        readonly property var chat: controllers.chats.current
        readonly property real listSpacing: 5
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: d.chat.title
        description: (!d.chat.isGroup && !d.chat.lastActivityText) ? " " : d.chat.lastActivityText
        showSeparator: !groupInvitationDialog.visible
        titleClickable: true

        contextMenuVisible: !groupInvitationDialog.visible
        contextMenu: ContextMenu {
            ContextMenuItem {
                text: d.chat.isGroup ? qsTr("Group info") : qsTr("Chat info")
                onTriggered: appState.requestInfo()
            }
        }

        onTitleClicked: appState.requestInfo()
    }

    footer: ChatMessageInput {
        id: footerControl
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

    Component.onCompleted: {
        appState.messageSent.connect(messageSentAudio.play)
    }
}

