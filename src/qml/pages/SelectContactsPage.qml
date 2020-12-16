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

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Search {
                id: contactSearch
                state: "opened"
                searchPlaceholder: qsTr("Search contact")
                closeable: false
                Layout.preferredHeight: d.defaultSearchHeight
                Layout.fillWidth: true
            }

            SelectContactsList {
                search: d.search
                newContactText: d.search ? d.contact : d.previousSearch
                Layout.fillWidth: true
                Layout.fillHeight: true

                onContactSelected: root.contactSelected(contactId)

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }

                    height: 1
                    color: Theme.chatBackgroundColor
                }
            }

            SelectedContactsFlow {
                visible: d.model.selection.hasSelection
                Layout.fillWidth: true
                Layout.preferredHeight: recommendedHeight

                Behavior on Layout.preferredHeight {
                    NumberAnimation {
                        easing.type: Easing.InOutCubic
                        duration: Theme.animationDuration
                    }
                }
            }
        }

        FormPrimaryButton {
            id: actionButton
            visible: d.model.selection.multiSelect
            enabled: d.model.selection.hasSelection
            onClicked: root.actionButtonClicked()
        }

        ServerSelectionRow {
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
