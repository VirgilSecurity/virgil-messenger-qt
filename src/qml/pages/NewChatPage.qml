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
    readonly property int modelCount: contactListView.count
    property string searchResultState: "search empty"
    property string serverName: "Default"

    readonly property int defaultMargins: 20
    readonly property int defaultSearchHeight: 40
    readonly property int defaultChatHeight: 50

    onSearchChanged: {
        if (filterSource) {
            filterSource.filter = search
            checkState()
        }
    }

    onModelCountChanged: {
        checkState()
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

    Item {
        id: searchResultsItem
        property real headerHeight: 20
        property real headerOpacity: 0

        anchors {
            top: contactSearch.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            topMargin: 1
            bottomMargin: defaultChatHeight
            margins: defaultMargins
        }

        state: searchResultState
        states: [
            State {
                name: "show header"
                PropertyChanges {
                    target: searchResultsItem
                    headerHeight: 90
                    headerOpacity: 1
                }
            },
            State {
                name: "hide header"
                PropertyChanges {
                    target: searchResultsItem
                    headerHeight: 20
                    headerOpacity: 0
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation {
                    properties: "headerHeight, headerOpacity"
                    easing.type: Easing.InExpo
                    duration: Theme.animationDuration
                }
            }
        ]

        ListView {
            id: contactListView
            signal placeholderClicked()

            anchors.fill: parent
            model: models.discoveredContacts.proxy
            spacing: 10
            clip: true
            header: contactListHeader
            delegate: contactListComponent
            footer: Item {
                width: width
                height: 20
            }
        }

        Component {
            id: contactListHeader

            Item {
                width: parent.width
                height: searchResultsItem.headerHeight
                opacity: searchResultsItem.headerOpacity

                ListDelegate {
                    id: contactListDelegate
                    anchors.centerIn: parent
                    width: parent.width
                    height: defaultChatHeight

                    Item {
                        width: avatar.width
                        height: parent.height

                        Avatar {
                            id: avatar
                            nickname: search.length === 0 ? previousSearch : contact
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        clip: true

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: search.length === 0 ? previousSearch : contact
                        }

                        Text {
                            color: Theme.secondaryTextColor
                            font.pointSize: UiHelper.fixFontSz(12)
                            text: qsTr("Click here to create chat")
                            width: parent.width
                            elide: Text.ElideRight
                            textFormat: Text.RichText
                        }
                    }
                }
            }
        }

        Component {
            id: contactListComponent

            Item {
                readonly property string name: model.name
                readonly property string avatarUrl: model.avatarUrl
                readonly property string lastSeenActivity: model.lastSeenActivity

                width: contactListView.width
                height: defaultChatHeight

                ListDelegate {
                    id: contactListDelegate
                    anchors.fill: parent

                    Item {
                        width: avatar.width
                        height: parent.height

                        Avatar {
                            id: avatar
                            nickname: name
                            avatarUrl: avatarUrl
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        clip: true

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: name
                        }

                        Text {
                            color: Theme.secondaryTextColor
                            font.pointSize: UiHelper.fixFontSz(12)
                            text: details
                            width: parent.width
                            elide: Text.ElideRight
                            textFormat: Text.RichText
                        }
                    }

                    onClicked: accept()
                }
            }
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

    function checkState() {
        if (search) {
            if (modelCount === 1 && search.toLowerCase() === contactListView.itemAtIndex(0).name.toLowerCase()) {
                searchResultState = "hide header"
            } else {
                searchResultState = "show header"
            }
            previousSearch = search

        } else {
            searchResultState = "hide header"
        }
    }

    function accept() {
        appState.addNewChat(contact)
    }

    function reject() {
        app.stateManager.goBack()
    }
}
