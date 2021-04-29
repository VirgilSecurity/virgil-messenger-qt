import QtQuick 2.15

MultiSelectContactsPage {
    appState: app.stateManager.cloudFolderMembersState

    signal selected(var contacts)

    Component.onCompleted: {
        appState.reload()
        appState.contactsSelected.connect(selected)
    }
}
