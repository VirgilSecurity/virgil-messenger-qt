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
    property string serverName: "Default"

    readonly property int defaultSearchHeight: 40
    readonly property int defaultChatHeight: 50
    readonly property int flowSpacing: 3
    readonly property int flowItemHeight: 30
    readonly property int lineWidth: 2

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

    CustomForm {
        id: form

        Item {
            id: addedContacts
            clip: true
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                leftMargin: Theme.margin
                rightMargin: Theme.margin
            }
            height: {
                if (addedContactsView.contentHeight > defaultChatHeight * 2) {
                    return defaultChatHeight * 2
                } else {
                    if (addedContactsView.count > 0) {
                        return addedContactsView.contentHeight
                    } else {
                        return 0
                    }
                }
            }

            Behavior on height {
                NumberAnimation {
                    easing.type: Easing.InOutCubic
                    duration: Theme.animationDuration
                }
            }

            FlowListView {
                id: addedContactsView
                anchors.fill: parent
                model: models.discoveredContacts.selectedContacts
                spacing: flowSpacing
                delegate: addedContactComponent
                focus: true
                onFocusChanged: {
                    if (!focus) {
                        addedContactsView.currentIndex = -1
                    }
                }
            }

            Component {
                id: addedContactComponent
                Rectangle {
                    id: contactRec
                    height: flowItemHeight
                    width: row.width + row.spacing
                    color: addedContactsView.currentIndex === index ? Theme.buttonPrimaryColor : Theme.contactPressedColor
                    radius: height

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            addedContactsView.focus = true
                            if (addedContactsView.currentIndex !== index) {
                                addedContactsView.currentIndex = index
                            } else {
                                addedContactsView.currentIndex = -1
                            }
                        }
                    }

                    Row {
                        id: row
                        spacing: Theme.smallSpacing
                        anchors.verticalCenter: parent.verticalCenter
                        Item {
                            height: flowItemHeight
                            width: height
                            Item {
                                height: flowItemHeight
                                width: height
                                visible: addedContactsView.currentIndex === index
                                enabled: addedContactsView.currentIndex === index
                                Repeater {
                                    model: 2
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 0.5 * parent.width
                                        height: lineWidth
                                        radius: height
                                        color: Theme.brandColor
                                        rotation: index ? -45 : 45
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        addedContactsView.currentIndex = -1
                                        addedContactsView.model.remove(index)
                                    }
                                }
                            }

                            Avatar {
                                id: avatar
                                nickname: model.name
                                avatarUrl: model.avatarUrl
                                diameter: flowItemHeight
                                anchors.verticalCenter: parent.verticalCenter
                                visible: addedContactsView.currentIndex !== index
                            }
                        }

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: model.name
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            Rectangle {
                id: separator
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: Theme.chatBackgroundColor
                anchors.bottom: parent.bottom
                visible: addedContactsView.count > 0
            }
        }

        Item {
            id: contactSearchItem
            anchors {
                top: addedContacts.bottom
                left: parent.left
                right: parent.right
                margins: Theme.margin
                topMargin: Theme.smallSpacing
            }
            height: contactSearch.height + Theme.smallSpacing

            Search {
                id: contactSearch
                height: defaultSearchHeight
                width: parent.width
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

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: Theme.chatBackgroundColor
                anchors.bottom: parent.bottom
                visible: selectedContacts.count > 0
            }
        }

        SelectContactsList {
            id: selectedContacts
            anchors {
                top: contactSearchItem.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                bottomMargin: defaultChatHeight
            }
        }

        RowLayout {
            anchors {
                top: selectedContacts.bottom
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

    ImageButton {
        image: "Arrow-Right"
        backgroundColor: Theme.contactPressedColor
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: Theme.margin
            bottomMargin: Theme.margin * 3
        }
    }

    function accept() {
    }

    function reject() {
        app.stateManager.goBack()
    }
}
