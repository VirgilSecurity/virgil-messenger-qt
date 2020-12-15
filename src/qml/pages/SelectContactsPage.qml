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

    QtObject {
        id: d
        readonly property var model: models.discoveredContacts
        readonly property string contact: contactSearch.search.toLowerCase()
        readonly property alias search: contactSearch.search
        property string previousSearch
        readonly property int defaultSearchHeight: 40

        onSearchChanged: {
            if (search) {
                previousSearch = search
            }
            model.filter = search
        }
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
            Layout.preferredHeight: d.defaultSearchHeight
            Layout.fillWidth: true

            onClosed: contactSearch.search = ""
            onAccepted: root.accept()
        }

        SelectContactsList {
            newContactText: d.search ? d.contact : d.previousSearch
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        SelectedContactsFlow {
            visible: d.model.selection.hasSelection
            Layout.fillWidth: true
            Layout.preferredHeight: recommendedHeight
        }

        ServerSelectionRow {
            Layout.alignment: Qt.AlignHCenter
        }

        FormPrimaryButton {
            id: actionButton
            visible: d.model.selection.multiSelect
            enabled: d.model.selection.hasSelection
        }
    }

    function accept() {
        if (!d.model.selection.multiSelect) {
            appState.addNewChat(d.contact)
        }
    }

    function reject() {
        app.stateManager.goBack()
    }
}
