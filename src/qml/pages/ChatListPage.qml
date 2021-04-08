import QtQuick 2.15
import QtQuick.Controls 2.15

import "../components"
import "../theme"

Page {
    id: root

    QtObject {
        id: d
        readonly property var state: app.stateManager.chatListState
        readonly property bool online: messenger.connectionStateString === "connected"
    }

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
            onTriggered: d.state.requestNewChat()
            enabled: d.online
        }

        ContextMenuItem {
            text: qsTr("New group")
            onTriggered: d.state.requestNewGroupChat()
            enabled: d.online
        }
    }

    ChatListView {
        searchHeader: root.header
        anchors.fill: parent

        onChatSelected: controllers.chats.openChat(chatId)
    }
}
