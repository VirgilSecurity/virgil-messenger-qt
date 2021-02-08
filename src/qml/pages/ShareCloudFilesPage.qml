import QtQuick 2.15

import "../components"

SelectContactsPage {
    appState: app.stateManager.shareCloudFilesState

    header: NewGroupChatHeader {
        title: qsTr("Share to")
        description: selectedContacts.count > 0 ? qsTr("%1 selected".arg(selectedContacts.count)) : ""
        rightButtonEnabled: selectedContacts.count > 0
        onActionButtonClicked: {
            controllers.cloudFiles.shareFiles()
            app.stateManager.goBack()
        }
    }
}
