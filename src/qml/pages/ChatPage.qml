import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QuickFuture 1.0

import "../theme"
import "../components"

Page {

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: Control {
        height: 60
        width: parent.width

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
                imageSource: "../resources/icons/Arrow-Left.png"
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
                    topPadding: 2
                    text: qsTr("Last seen yesterday")
                    font.pointSize: UiHelper.fixFontSz(12)
                    color: Theme.secondaryTextColor
                }
            }

            SettingsButton {
                Layout.alignment: Qt.AlignRight
                Action { text: qsTr("Item 1") }
                Action { text: qsTr("Item 2") }
            }
        }
    }

    ListView {
        id: listView

        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        verticalLayoutDirection: ListView.BottomToTop
        spacing: 12
        model: ConversationsModel
        delegate: ChatMessage {
            text: message
            author: model.author === "Me"
                      ? Messenger.currentUser
                      : model.author
            timeStamp: model.timestamp
            variant: model.author === "Me"
                      ? "light"
                      : "dark"
        }

        ScrollBar.vertical: ScrollBar {
            bottomPadding: 5
        }
    }

    footer: ChatMessageInput {
        onMessageSending: {
            var future = Messenger.sendMessage(ConversationsModel.recipient, message)
            Future.onFinished(future, function(value) {
              console.log("Send message result: ", Future.result(future))
            })
        }
    }
}

