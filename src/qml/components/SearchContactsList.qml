import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../base"
import "../components"
import "../theme"

Item {
    id: searchContactsList
    property real headerHeight: 20
    property real headerOpacity: 0
    readonly property real expandedHeaderHeight: 90
    property alias contactListView: contactListView
    property alias modelCount: contactListView.count
    property alias model: contactListView.model

    state: {
        if (search !== controllers.users.userId) {
            if (models.discoveredContacts.newContactFiltered) {
                return "show header"
            } else {
                return "hide header"
            }
        }
        return "hide header"
    }

    onStateChanged: {
        if (state === "show header") {
            flickListView.restart()
        }
    }

    states: [
        State {
            name: "show header"
            PropertyChanges {
                target: searchContactsList
                headerHeight: expandedHeaderHeight
                headerOpacity: 1
            }
        },
        State {
            name: "hide header"
            PropertyChanges {
                target: searchContactsList
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

    PropertyAnimation {
        id: flickListView
        target: contactListView
        property: "contentY"
        to: -expandedHeaderHeight
        duration: Theme.animationDuration
    }

    ListView {
        id: contactListView
        signal placeholderClicked()

        anchors.fill: parent
        spacing: Theme.smallSpacing
        clip: true
        header: contactListHeader
        delegate: contactListComponent
        focus: true
        footer: Item {
            width: width
            height: Theme.spacing
        }
    }

    Component {
        id: contactListHeader

        Item {
            width: parent.width
            height: searchContactsList.headerHeight
            opacity: searchContactsList.headerOpacity
            enabled: state == "show header"

            ListDelegate {
                id: contactListDelegate
                anchors.centerIn: parent
                width: parent.width
                height: defaultChatHeight

                Avatar {
                    id: avatar
                    nickname: search ? contact : previousSearch
                    Layout.alignment: Qt.AlignVCenter
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

                onClicked: accept()
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

                Avatar {
                    id: avatar
                    nickname: model.name
                    avatarUrl: model.avatarUrl
                    Layout.alignment: Qt.AlignVCenter
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
                        text: model.lastSeenActivity
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
