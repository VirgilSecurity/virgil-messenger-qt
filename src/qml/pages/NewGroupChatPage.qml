import QtQuick 2.15

import "../components"

SelectContactsPage {
    appState: app.stateManager.newGroupChatState

    header: PageHeader {
        title: qsTr("Add members")
        description: selectedContacts.count ? qsTr("%1 selected".arg(selectedContacts.count)) : ""
        titleHorizontalAlignment: Qt.AlignHCenter

        rightControl: ImageButton {
            id: rightButton
            image: "Send"
            visible: selectedContacts.count
            onClicked: appState.requestChatName()
        }
    }
}
