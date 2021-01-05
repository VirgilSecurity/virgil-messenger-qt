import QtQuick 2.15

import "../components"

SelectContactsPage {
    appState: app.stateManager.newGroupChatState
    actionButtonText: qsTr("Next")

    header: Header {
        title: qsTr("New group")
    }

    onActionButtonClicked: appState.requestChatName()
}
