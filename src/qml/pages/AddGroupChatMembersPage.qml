import QtQuick 2.15

MultiSelectContactsPage {
    appState: app.stateManager.addGroupChatMembersState
    onFinished: appState.addMembers()
}
