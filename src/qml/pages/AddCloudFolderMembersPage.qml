import QtQuick 2.15

MultiSelectContactsPage {
    appState: app.stateManager.addCloudFolderMembersState
    onFinished: appState.addMembers()
}
