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

    delegate: ChatListDelegate {
        width: chatListView.width
        onSelectItem: chatListView.chatSelected(model.id)
    }

    onPlaceholderClicked: appState.requestNewChat()
}
