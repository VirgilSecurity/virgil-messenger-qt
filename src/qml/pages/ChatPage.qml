import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QuickFuture 1.0
import QtQuick.Window 2.12
import QtMultimedia 5.12
import com.virgilsecurity.messenger 1.0

import "../theme"
import "../components"

Page {

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
                    text: messenger.recipient
                    font.pointSize: UiHelper.fixFontSz(15)
                    color: Theme.primaryTextColor
                    font.bold: true
                }

                Label {
                    topPadding: 2
                    text: qsTr("Last seen yesterday")
                    font.pointSize: UiHelper.fixFontSz(12)
                    color: Theme.secondaryTextColor
                }
            }
        }
    }


    ListView {
        id: listView

        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        section.property: "day"
        section.delegate: ChatDateSeporator {
            date: section
        }

        spacing: 5
        // FIXME(fpohtmeh): restore
        //model: ConversationsModel
        delegate: ChatMessage {
            readonly property bool isUser: model.author === Enums.MessageAuthor.User;
            text: model.message
            nickname: isUser ? Messenger.currentUser : Messenger.currentRecipient
            timeStamp: model.timestamp
            variant: isUser ? "light" : "dark"
            messageInARow: model.messageInARow
            firstMessageInARow: model.firstMessageInARow
            status: isUser ? model.status :  "none"
            attachmentId: model.attachmentId
            attachmentSize: model.attachmentSize
            attachmentType: model.attachmentType
            attachmentLocalUrl: model.attachmentLocalUrl
            attachmentLocalPreview: model.attachmentLocalPreview
        }

        onCountChanged: {
            positionViewAtEnd()
        }

        ScrollBar.vertical: ScrollBar { }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                forceActiveFocus()
                mouse.accepted = false
            }
        }
    }

    footer: ChatMessageInput {
        id: footerControl
        onMessageSending: messenger.createSendMessage(message, attachmentUrl, attachmentType)
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

    Connections {
        target: messenger
        onSendMessageSuccess: messageSent.play
        onSendMessageError: messageSent.play
    }
}

