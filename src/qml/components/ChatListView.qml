import QtQuick 2.15

ModelListView {
    id: chatListView
    emptyIcon: "../resources/icons/Chats.png"
    emptyText: qsTr("Create your first chat<br/>by pressing the dots<br/>button above")

    signal chatSelected(string chatId)

    delegate: ChatListDelegate {
        width: chatListView.width

        onClicked: chatSelected(model.id)
    }
}
