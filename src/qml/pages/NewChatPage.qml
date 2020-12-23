import QtQuick 2.15

import "../components"

SelectContactsPage {
    appState: app.stateManager.newChatState

    header: Header {
        title: qsTr("New chat")
    }

    onContactSelected: appState.addNewChat(contactId)
}
