import QtQuick 2.15

import "../components"

SelectContactsPage {
    appState: app.stateManager.newGroupChatState
    actionButtonText: qsTr("Create group")

    header: Header {
        title: qsTr("New group")
    }
}
