import QtQuick 2.15

MultiSelectContactsPage {
    appState: app.stateManager.addCloudFolderMembersState
    buttonText: qsTr("Add members")
}
