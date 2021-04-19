import QtQuick 2.15
import QtQuick.Controls 2.15

import "../components"
import "../theme"

Rectangle {
    color: Theme.contactsBackgroundColor

    QtObject {
        id: d

        readonly property var manager: app.stateManager
        readonly property var chatListState: manager.chatListState
        readonly property bool online: messenger.connectionStateString === "connected"
    }

    Page {
        id: page

        anchors.fill: parent
        anchors.leftMargin: Theme.smallMargin
        anchors.rightMargin: Theme.smallMargin

        background: Rectangle {
            color: Theme.contactsBackgroundColor
        }

        header: SearchHeader {
            title: app.organizationDisplayName
            description: qsTr("%1 Server").arg(app.organizationDisplayName)
            searchPlaceholder: qsTr("Search conversation")
            filterSource: models.chats

            ContextMenuItem {
                text: qsTr("New chat")
                onTriggered: d.chatListState.requestNewChat()
                enabled: d.online
            }

            ContextMenuItem {
                text: qsTr("New group")
                onTriggered: d.chatListState.requestNewGroupChat()
                enabled: d.online
            }
        }

        ChatListView {
            searchHeader: page.header
            anchors.fill: parent

            onChatSelected: controllers.chats.openChat(chatId)
            onChatDeselected: d.manager.goBack()
        }
    }
}
