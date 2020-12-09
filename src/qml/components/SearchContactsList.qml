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
    property alias contactListView: contactListView
    property alias modelCount: contactListView.count

    anchors {
        top: contactSearch.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom
        topMargin: 1
        bottomMargin: defaultChatHeight
        margins: defaultMargins
    }

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

    states: [
        State {
            name: "show header"
            PropertyChanges {
                target: searchContactsList
                headerHeight: 90
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
            height: searchContactsList.headerHeight
            opacity: searchContactsList.headerOpacity
            enabled: state == "show header"

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
                        nickname: search ? contact : previousSearch
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

                Item {
                    width: avatar.width
                    height: parent.height

                    Avatar {
                        id: avatar
                        nickname: model.name
                        avatarUrl: model.avatarUrl
                        anchors.verticalCenter: parent.verticalCenter
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
                        text: model.details
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
