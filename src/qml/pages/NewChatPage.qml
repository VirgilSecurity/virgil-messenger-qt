import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import "../base"
import "../components"
import "../theme"

OperationPage {
    appState: app.stateManager.newChatState
    loadingText: qsTr("Adding of contact...")
    footerText: ""
    readonly property string contact: contactSearch.search.toLowerCase()
    property var filterSource: models.discoveredContacts
    property alias search: contactSearch.search
    property int modelCount: contactListView.count
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
//        showBackButton: !form.isLoading
        showBackButton: true
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
                name: "not found" // search word is NOT IN model
                PropertyChanges {
                    target: searchResultsItem
                    headerHeight: 90
                    headerOpacity: 1
                }
            },
            State {
                name: "found" // search word is IN model
                PropertyChanges {
                    target: searchResultsItem
                    headerHeight: 20
                    headerOpacity: 0
                }
            },
            State {
                name: "search empty" // search is empty
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
                    duration: Theme.defaultAnimationDuration
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
                            nickname: contact
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        clip: true

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text:  contact
                        }

                        Text {
                            color: Theme.secondaryTextColor
                            font.pointSize: UiHelper.fixFontSz(12)
                            text: "Click here to create chat"
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
                            nickname: model.name
                            anchors.verticalCenter: parent.verticalCenter

                            Item {
                                id: imageItem
                                anchors.fill: parent
                                layer.enabled: true
                                layer.effect: OpacityMask {
                                    maskSource: Item {
                                        width: avatar.width
                                        height: avatar.height
                                        Rectangle {
                                            anchors.fill: parent
                                            radius: parent.height
                                        }
                                    }
                                }

                                Image {
                                    anchors.fill: parent
                                    source: model.imageSource
                                    mipmap: true
                                    asynchronous: true
                                    fillMode: Image.PreserveAspectCrop
                                }
                            }
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        clip: true

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: model.name
                        }

                        Text {
                            color: Theme.secondaryTextColor
                            font.pointSize: UiHelper.fixFontSz(12)
                            text: model.status
                            width: parent.width
                            elide: Text.ElideRight
                            textFormat: Text.RichText
                        }
                    }

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
            formSpacing: 25
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
            if (modelCount === 1) {
                searchResultState = "found"
            } else {
                searchResultState = "not found"
            }
        } else {
            searchResultState = "search empty"
        }
    }

    function accept() {
        appState.addNewChat(contact)
    }

    function reject() {
        app.stateManager.goBack()
    }
}
