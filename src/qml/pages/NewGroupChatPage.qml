import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../base"
import "../components"
import "../theme"

OperationPage {
    appState: app.stateManager.newGroupChatState
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
        title: qsTr("New group")
    }

    Item {
        id: addedContacts
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: defaultMargins
            rightMargin: defaultMargins
        }
        height: defaultChatHeight * 2
    }

    Search {
        id: contactSearch
        anchors {
            top: addedContacts.bottom
            left: parent.left
            right: parent.right
            margins: defaultMargins
            topMargin: 0
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
//        model: models.discoveredContacts.proxy
        model: tempModel
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
//        appState.addNewChat(contact)

    }

    function reject() {
        app.stateManager.goBack()
    }

    // TEMP

    ListModel {
        id: tempModel

        ListElement {
            name: "John Doe"
            avatarUrl: "https://avatars.mds.yandex.net/get-zen_doc/1779726/pub_5d32ac8bf2df2500adb00103_5d32aeae21f9ff00ad9973ee/scale_1200"
            details: "yesterday"
        }
        ListElement {
            name: "Bon Min"
            avatarUrl: "https://peopletalk.ru/wp-content/uploads/2016/10/orig_95f063cefa53daf194fa9f6d5e20b86c.jpg"
            details: "yesterday"
        }
        ListElement {
            name: "Tin Bin"
            avatarUrl: "https://i.postimg.cc/wBJKr6CR/K5-W-z1n-Lqms.jpg"
            details: "yesterday"
        }
        ListElement {
            name: "Mister Bean"
            avatarUrl: "https://avatars.mds.yandex.net/get-zen_doc/175962/pub_5a7b1334799d9dbfb9cc0f46_5a7b135b57906a1b6eb710eb/scale_1200"
            details: "yesterday"
        }
        ListElement {
            name: "Erick Helicopter"
            avatarUrl: ""
            details: "yesterday"
        }
        ListElement {
            name: "Peter Griffin"
            avatarUrl: ""
            details: "yesterday"
        }
    }
}
