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

    property alias actionButtonText: actionButton.text

    signal contactSelected(string contactId)
    signal actionButtonClicked()

    QtObject {
        id: d
        readonly property var model: models.discoveredContacts
        readonly property string contact: contactSearch.search.toLowerCase()
        readonly property alias search: contactSearch.search

        onSearchChanged: model.filter = search
    }

    header: Header {
        title: qsTr("New group")
    }

    Form {
        id: form
        isCentered: false

        Search {
            id: contactSearch
            state: "opened"
            searchPlaceholder: qsTr("Search contact")
            closeable: false
            Layout.preferredHeight: recommendedHeight
            Layout.fillWidth: true
            Layout.leftMargin: Theme.smallSpacing
        }

        SelectContactsList {
            search: d.search
            Layout.fillWidth: true
            Layout.fillHeight: true

            footer: HorizontalRule {}
            footerPositioning: ListView.OverlayFooter

            onContactSelected: root.contactSelected(contactId)
        }

        SelectedContactsFlow {
            id: flow
            Layout.fillWidth: true
            Layout.preferredHeight: recommendedHeight

            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    easing.type: Easing.InOutCubic
                    duration: Theme.animationDuration
                }
            }
        }

        FormPrimaryButton {
            id: actionButton
            visible: d.model.selection.multiSelect
            enabled: flow.visible
            onClicked: root.actionButtonClicked()
        }

        ServerSelectionRow {
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
