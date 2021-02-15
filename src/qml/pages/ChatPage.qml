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
        readonly property real listSpacing: 5
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: controllers.chats.current.title
        description: appState.lastActivityText
        contextMenuVisible: !groupInvitationDialog.visible
        showSeparator: !groupInvitationDialog.visible
        contextMenu: ContextMenu {
            ContextMenuItem {
                text: qsTr("Call")
                iconName: "Make-Call"
                visible: false
            }

            ContextMenuItem {
                text: appState.isGroupChat ? qsTr("Group info") : qsTr("Chat info")
                onTriggered: appState.requestInfo()
            }

            ContextMenuSeparator {
                visible: appState.isGroupChat
            }

            ContextMenuItem {
                text: qsTr("Add participant")
                visible: appState.isGroupChat
                onTriggered: controllers.chats.addParticipant("userId")
            }

            ContextMenuItem {
                text: qsTr("Remove participant")
                visible: appState.isGroupChat
                onTriggered: controllers.chats.removeParticipants("userId")
            }

            ContextMenuItem {
                text: qsTr("Leave group")
                visible: appState.isGroupChat
                onTriggered: controllers.chats.leaveGroup()
            }
        }
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

