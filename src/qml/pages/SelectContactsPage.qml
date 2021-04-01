import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

OperationPage {
    id: root
    loadingText: qsTr("Adding of contact...")
    footerText: ""

    property alias selectedContacts: flow
    property alias showButton: button.visible
    property alias buttonText: button.text

    QtObject {
        id: d
        readonly property var model: models.discoveredContacts
        readonly property string contact: contactSearch.search.toLowerCase()
        readonly property alias search: contactSearch.search

        onSearchChanged: model.filter = search

        function selectContact(contactUsername) {
            appState.contactSelected(contactUsername)
            contactSearch.clear()
            if (models.discoveredContacts.selection.multiSelect) {
                models.discoveredContacts.toggleByUsername(contactUsername)
            }
        }
    }

    Form {
        id: form
        isCentered: false

        ContactsFlowView {
            id: flow
            Layout.fillWidth: true
            Layout.preferredHeight: recommendedHeight
            visible: d.model.selection.multiSelect

            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    easing.type: Easing.InOutCubic
                    duration: Theme.animationDuration
                }
            }

            onContactSelected: d.selectContact(contactUsername)
        }

        Search {
            id: contactSearch
            state: "opened"
            searchPlaceholder: qsTr("Search contact")
            textValidator: app.validator.reUsername // TODO(fpohtmeh): delete once we have contact discover
            closeable: false

            Layout.preferredHeight: recommendedHeight
            Layout.fillWidth: true
            Layout.leftMargin: Theme.smallSpacing

            onAccepted: {
                const username = models.discoveredContacts.firstContactUsername()
                if (username) {
                    d.selectContact(username)
                }
            }
        }

        ContactsListView {
            id: contactsList
            search: d.search
            Layout.fillWidth: true
            Layout.fillHeight: true

            footer: HorizontalRule {}
            footerPositioning: ListView.OverlayFooter

            onContactSelected: d.selectContact(contactUsername)
        }

        ServerSelectionRow {
            Layout.alignment: Qt.AlignHCenter
        }

        FormPrimaryButton {
            id: button
            text: qsTr("Select")
            visible: false
            enabled: selectedContacts.count
            onClicked: appState.finishSelection()
        }
    }
}
