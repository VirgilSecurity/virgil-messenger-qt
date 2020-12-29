import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtMultimedia 5.15

import "../base"
import "../theme"
import "../components"

Page {
    id: chatPage

    readonly property var appState: app.stateManager.chatState
    readonly property var chatName: controllers.chats.currentChatName
    readonly property var contactId: controllers.chats.currentContactId
    property real chatListViewHeight: 0

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
                    text: chatName
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

            ImageButton {
                image: "More"
                onClicked: {
                    if (appState.isGroupChat) {
                        if (appState.isAdmin) {
                            groupChatAdminContextMenu.open()
                        } else {
                            groupChatNotAdminContextMenu.open()
                        }
                    } else {
                        stdChatContextMenu.open()
                    }
                }

                ContextMenu {
                    id: stdChatContextMenu
                    dropdown: true
                    currentIndex: -1

                    Action {
                        text: qsTr("Delete chat")
                        onTriggered: controllers.chats.addParticipant("userId")
                    }
                }

                ContextMenu {
                    id: groupChatNotAdminContextMenu
                    dropdown: true
                    currentIndex: -1

                    Action {
                        text: qsTr("Leave group")
                        onTriggered: controllers.chats.leaveGroup()
                    }
                }

                ContextMenu {
                    id: groupChatAdminContextMenu
                    dropdown: true
                    currentIndex: -1

                    Action {
                        text: qsTr("Add participant")
                        onTriggered: controllers.chats.addParticipant("userId")
                    }

                    Action {
                        text: qsTr("Remove participant")
                        onTriggered: controllers.chats.removeParticipants("userId")
                    }

                    Action {
                        text: qsTr("Leave group")
                        onTriggered: controllers.chats.leaveGroup()
                    }
                }
            }
        }
    }

    footer: ChatMessageInput {
        id: footerControl
    }

    ChatItem {
        anchors.fill: parent
    }

// Components

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

