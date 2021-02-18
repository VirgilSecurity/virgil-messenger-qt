import QtQuick 2.15

import "../components"

SelectContactsPage {
    id: selectContactsPage
    appState: app.stateManager.addGroupChatMembersState

    header: PageHeader {
        title: qsTr("Add members")
        description: selectedContacts.count ? qsTr("%1 selected".arg(selectedContacts.count)) : ""
        titleHorizontalAlignment: Qt.AlignHCenter

        rightControl: ImageButton {
            id: rightButton
            image: "Send"
            visible: selectedContacts.count
            onClicked: appState.addMembers()
        }
    }
}
