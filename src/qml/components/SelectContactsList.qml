import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Item {
    id: root
    state: model.newContactFiltered ? "show header" : "hide header"

    property real headerHeight: 20
    property real headerOpacity: 0
    readonly property real expandedHeaderHeight: 90
    readonly property int checkCircleMargin: 2
    property var model: models.discoveredContacts

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

        ListDelegate {
            id: contactListDelegate
            width: contactListView.width
            height: defaultChatHeight

            Item {
                width: parent.width
                height: parent.height
                Row {
                    width: parent.width
                    height: parent.height
                    spacing: Theme.spacing

                    add: Transition {
                        NumberAnimation { property: "scale"; from: 0.9; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
                        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
                    }

                    move: Transition {
                        NumberAnimation { properties: "x,y"; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
                    }

                    Rectangle {
                        width: headerHeight
                        height: width
                        radius: height
                        color: Theme.avatarBgColor
                        visible: model.isSelected
                        anchors.verticalCenter: parent.verticalCenter

                        Repeater {
                            model: 2

                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width * 0.6
                                height: checkCircleMargin
                                radius: height
                                rotation: index ? 0 : 90
                            }
                        }
                    }

                    Avatar {
                        id: avatar
                        nickname: model.name
                        avatarUrl: model.avatarUrl
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter

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
