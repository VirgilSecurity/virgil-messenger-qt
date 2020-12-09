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
    readonly property int modelCount: searchResultsItem.modelCount
    property string serverName: "Default"

    readonly property int defaultMargins: 20
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

    Search {
        id: contactSearch
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: defaultMargins
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

    SearchContactsList {
        id: searchResultsItem
        anchors {
            top: contactSearch.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            topMargin: 1
            bottomMargin: defaultChatHeight
            margins: defaultMargins
        }
    }

    Item {
        anchors {
            top: searchResultsItem.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: defaultMargins
            rightMargin: defaultMargins
        }

        Form {
            id: form
            formSpacing: Theme.spacing
            formLogo.visible: false
            focus: true

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumWidth: Theme.formMaximumWidth
                Layout.alignment: Qt.AlignHCenter

                Label {
                    text: qsTr("Server")
                    color: Theme.primaryTextColor
                    Layout.fillWidth: true
                }

                Label {
                    color: Theme.secondaryTextColor
                    text: serverName
                }
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
