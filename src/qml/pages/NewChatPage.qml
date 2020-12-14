import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../base"
import "../components"
import "../theme"

OperationPage {
    appState: app.stateManager.newChatState
    loadingText: qsTr("Adding of contact...")
    footerText: ""
    readonly property string contact: contactSearch.search.toLowerCase()
    readonly property var filterSource: models.discoveredContacts
    readonly property alias search: contactSearch.search
    property string previousSearch
    readonly property int modelCount: searchResultsItem.count
    property string serverName: "Default"

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
        title: qsTr("New chat")
    }

    CustomForm {
        id: form

        Search {
            id: contactSearch
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: Theme.margin
            }
            height: defaultSearchHeight
            state: "opened"
            searchPlaceholder: qsTr("Search contact")
            onClosed: {
                contactSearch.search = ""
                reject()
            }
            onAccepted: {
                accept()
            }
        }

        SelectContactsList {
            id: searchResultsItem
            anchors {
                top: contactSearch.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                topMargin: 1
                bottomMargin: defaultChatHeight
            }
            model: models.discoveredContacts.proxy
        }

        RowLayout {
            anchors {
                top: searchResultsItem.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            Label {
                text: qsTr("Server")
                color: Theme.primaryTextColor
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                text: serverName
                color: Theme.secondaryTextColor
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    function accept() {
        appState.addNewChat(contact)
    }

    function reject() {
        app.stateManager.goBack()
    }
}
