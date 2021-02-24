import QtQuick 2.15

import "../components"

SelectContactsPage {
    id: root

    property alias headerTitle: pageHeader.title

    signal finished()

    header: PageHeader {
        id: pageHeader
        title: qsTr("Add members")
        description: selectedContacts.count ? qsTr("%1 selected".arg(selectedContacts.count)) : ""
        titleHorizontalAlignment: Qt.AlignHCenter

        rightControl: ImageButton {
            image: "Send"
            visible: selectedContacts.count
            onClicked: root.finished()
        }
    }
}
