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
        model: messenger.messages
        delegate: ChatMessage {
            body: model.body
            time: model.time
            nickname: model.nickname
            isUser: model.isUser
            status: model.status
            failed: model.failed

            inRow: model.inRow
            firstInRow: model.firstInRow

            attachmentId: model.attachmentId
            attachmentSize: model.attachmentSize
            attachmentDisplaySize: model.attachmentDisplaySize
            attachmentType: model.attachmentType
            attachmentLocalUrl: model.attachmentLocalUrl
            attachmentLocalPreview: model.attachmentLocalPreview
            attachmentUploaded: model.attachmentUploaded
            attachmentLoadingFailed: model.attachmentLoadingFailed
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
        onMessageSent: messageSent.play
        onSendMessageFailed: messageSent.play
    }
}

