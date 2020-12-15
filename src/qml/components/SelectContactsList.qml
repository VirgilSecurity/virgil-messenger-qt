import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Item {
    id: root
    state: d.model.filterHasNewContact ? "show header" : "hide header"

    property string newContactText: ""

    signal contactSelected(string contactId)

    QtObject {
        id: d

        readonly property var model: models.discoveredContacts
        property real headerHeight: defaultChatHeight
        property real headerOpacity: 0
        readonly property real defaultChatHeight: 50
        readonly property real defaultHeaderHeight: 20
        readonly property real expandedHeaderHeight: 90
        readonly property real checkCircleMargin: 2
        readonly property real selectionIconSize: 20
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
                target: d
                headerHeight: d.expandedHeaderHeight
                headerOpacity: 1
            }
        },
        State {
            name: "hide header"
            PropertyChanges {
                target: d
                headerHeight: d.defaultHeaderHeight
                headerOpacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                target: d
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
        to: -d.expandedHeaderHeight
        duration: Theme.animationDuration
    }

    ListView {
        id: contactListView

        signal placeholderClicked()

        anchors.fill: parent
        spacing: Theme.smallSpacing
        clip: true
        focus: true

        model: d.model.proxy
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
            height: d.headerHeight
            opacity: d.headerOpacity
            enabled: state == "show header"

            ListDelegate {
                id: contactListDelegate
                anchors.centerIn: parent
                width: parent.width
                height: d.defaultChatHeight

                Avatar {
                    id: avatar
                    nickname: root.newContactText
                    Layout.alignment: Qt.AlignVCenter
                }

                Column {
                    Layout.fillWidth: true
                    clip: true

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: root.newContactText
                    }

                    Text {
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        text: qsTr("Click here to select contact")
                        width: parent.width
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                    }
                }

                onClicked: {
                    if (d.model.selection.multiSelect) {
                        d.model.toggleById(root.newContactText)
                    }
                    contactSelected(root.newContactText)
                }
            }
        }
    }

    Component {
        id: contactListComponent

        ListDelegate {
            id: contactListDelegate
            width: contactListView.width
            height: d.defaultChatHeight

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

                // TODO(fpohtmeh): replace with image
                Item {
                    width: d.selectionIconSize
                    height: width
                    visible: d.model.selection.hasSelection
                    anchors.verticalCenter: parent.verticalCenter

                    Repeater {
                        model: isSelected ? 2 : 0

                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width * 0.6
                            height: d.checkCircleMargin
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

            onClicked: {
                if (d.model.selection.multiSelect) {
                    d.model.selection.toggle(index)
                }
                contactSelected(contactId)
            }
        }

    }
}
