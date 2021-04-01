import QtQuick 2.15

import "../components"

SelectContactsPage {
    id: root
    showButton: true

    property alias headerTitle: pageHeader.title

    header: PageHeader {
        id: pageHeader
        title: qsTr("Add members")
        description: selectedContacts.count ? qsTr("%1 selected".arg(selectedContacts.count)) : ""
        titleHorizontalAlignment: Qt.AlignHCenter
    }
}
