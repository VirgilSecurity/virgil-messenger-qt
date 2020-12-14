import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../base"
import "../components"
import "../theme"

Item {
    id: root
    property var model: models.discoveredContacts

    property real headerHeight: 20
    property real headerOpacity: 0
    readonly property real expandedHeaderHeight: 90
    readonly property int checkCircleMargin: 2

    state: {
        // FIXME(fpohtmeh): move this check to C++
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
                target: root
                headerHeight: expandedHeaderHeight
                headerOpacity: 1
            }
        },
        State {
            name: "hide header"
            PropertyChanges {
                target: root
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
        focus: true

        model: root.model.proxy
        header: contactListHeader
        delegate: contactListComponent
        footer: Item {
            width: width
            height: Theme.spacing
        }
    }

    Component {
        id: contactListHeader

        Item {
            width: parent.width
            height: root.headerHeight
            opacity: root.headerOpacity
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

                    Rectangle {
                        anchors {
                            right: parent.right
                            bottom: parent.bottom
                        }
                        width: parent.width * 0.4
                        height: width
                        radius: width
                        color: Theme.mainBackgroundColor
                        visible: model.isSelected

                        Rectangle {
                            anchors {
                                fill: parent
                                margins: checkCircleMargin
                            }
                            radius: width
                            color: Theme.contactPressedColor

                            Repeater {
                                model: 2

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width - 2 * checkCircleMargin
                                    height: checkCircleMargin
                                    radius: height
                                    rotation: index ? 0 : 90
                                }
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
                        text: model.details
                        width: parent.width
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                    }
                }

                onClicked: {
                    if (root.model.selection.multiSelect) {
                        root.model.selection.toggle(index)
                    }
                    else {
                        accept()
                    }
                }
            }
        }
    }
}
