import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

OperationPage {
    id: root
    appState: app.stateManager.newGroupChatState
    loadingText: qsTr("Adding of contact...")
    footerText: ""

    signal contactSelected(string contactId)
    signal actionButtonClicked()

    property alias selectedContacts: flow

    QtObject {
        id: d
        readonly property var model: models.discoveredContacts
        readonly property string contact: contactSearch.search.toLowerCase()
        readonly property alias search: contactSearch.search

        onSearchChanged: model.filter = search
    }

    Form {
        id: form
        isCentered: false

        SelectedContactsFlow {
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
                const contactId = d.model.firstContactId();
                if (contactId) {
                    root.contactSelected(contactId)
                }
            }
        }

        SelectContactsList {
            id: contactsList
            search: d.search
            Layout.fillWidth: true
            Layout.fillHeight: true

            footer: HorizontalRule {}
            footerPositioning: ListView.OverlayFooter

            onContactSelected: root.contactSelected(contactId)
        }

        ServerSelectionRow {
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
