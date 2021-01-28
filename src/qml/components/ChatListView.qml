import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"

ModelListView {
    id: chatListView
    model: d.model.proxy
    emptyIcon: "../resources/icons/Chats.png"
    emptyText: qsTr("Create your first chat<br/>by pressing the dots<br/>button above")

    signal chatSelected(string chatId)

    QtObject {
        id: d
        readonly property var model: models.chats
    }

    delegate: ListDelegate {
        width: chatListView.width
        backgroundColor: (model.isSelected || down) ? Theme.contactPressedColor : "transparent"

        Avatar {
            id: avatar
            nickname: model.contactId
        }

        Column {
            Layout.fillWidth: true
            clip: true

            Text {
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(15)
                text: model.contactId
            }

            Text {
                color: Theme.secondaryTextColor
                font.pointSize: UiHelper.fixFontSz(12)
                width: parent.width
                text: model.lastMessageBody
                elide: Text.ElideRight
            }
        }

        Column {
            width: 30
            spacing: 5

            MessageCounter {
               count: model.unreadMessageCount
               anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: model.lastEventTime
                color: Theme.secondaryTextColor
                font.pointSize: UiHelper.fixFontSz(9)
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        onClicked: {
            if (d.model.selection.multiSelect) {
                d.model.toggleById(model.id)
            }
            chatListView.chatSelected(model.id)
        }
    }

    onPlaceholderClicked: appState.requestNewChat()
}
