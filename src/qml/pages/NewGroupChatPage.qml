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

    readonly property var model: models.discoveredContacts
    // FIXME(fpohtmeh): remove superfluous properties
    readonly property string contact: contactSearch.search.toLowerCase()
    readonly property var filterSource: models.discoveredContacts
    readonly property alias search: contactSearch.search
    property string previousSearch
    readonly property int defaultSearchHeight: 40
    readonly property int defaultChatHeight: 50

    onSearchChanged: {
        if (search) {
            previousSearch = search
        }
        if (filterSource) {
            filterSource.filter = search
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
            Layout.preferredHeight: defaultSearchHeight
            Layout.fillWidth: true

            onClosed: {
                contactSearch.search = ""
                reject()
            }
            onAccepted: {
                accept()
            }
        }

        SelectedContactsFlow {
            model: root.model.selectedContacts
            Layout.fillWidth: true
            Layout.preferredHeight: flowItemHeight
        }

        SelectContactsList {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ServerSelectionRow {
        }

        FormPrimaryButton {
            text: qsTr("Create group")
            visible: root.model.selection.multiSelect
            enabled: root.model.selection.hasSelection
        }
    }

    function accept() {
    }

    function reject() {
        app.stateManager.goBack()
    }
}
