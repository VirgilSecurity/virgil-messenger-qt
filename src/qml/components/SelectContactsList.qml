import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

ModelListView {
    id: modelListView
    state: d.model.filterHasNewContact ? "show header" : "hide header"
    model: d.model.proxy
    header: contactListHeader
    delegate: contactListComponent

    property string newContactText: ""

    signal contactSelected(string contactId)

    QtObject {
        id: d

        readonly property var model: models.discoveredContacts
        property real headerHeight: defaultChatHeight
        property real headerOpacity: 0
        readonly property real defaultChatHeight: 50
        readonly property real defaultHeaderHeight: 20
        readonly property real expandedHeaderHeight: 70
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
        target: modelListView
        property: "contentY"
        to: -d.expandedHeaderHeight
        duration: Theme.animationDuration
        easing.type: Easing.InExpo
    }

    Component {
        id: contactListHeader

        Item {
            width: parent.width
            height: d.headerHeight
            opacity: d.headerOpacity
            enabled: modelListView.state === "show header"

            ListDelegate {
                id: contactListDelegate
                anchors.centerIn: parent
                width: parent.width
                height: d.defaultChatHeight

                Avatar {
                    id: avatar
                    nickname: modelListView.newContactText
                    Layout.alignment: Qt.AlignVCenter
                }

                Column {
                    Layout.fillWidth: true
                    clip: true

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: modelListView.newContactText
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
                        d.model.toggleById(modelListView.newContactText)
                    }
                    contactSelected(modelListView.newContactText)
                }
            }
        }
    }

    Component {
        id: contactListComponent

        ListDelegate {
            id: contactListDelegate
            leftInset: 0
            rightInset: 0
            width: modelListView.width
            height: d.defaultChatHeight

            Item { // we need this item!
                width: parent.width
                height: parent.height

                Row {
                    width: parent.width
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    spacing: Theme.smallSpacing

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
                        visible: isSelected ? true : false
                        anchors.verticalCenter: parent.verticalCenter

                        Image {
                            id: image
                            anchors.fill: parent
                            source: "../resources/icons/Check.png"
                            fillMode: Image.PreserveAspectFit
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
                if (d.model.selection.multiSelect) {
                    d.model.selection.toggle(index)
                }
                contactSelected(contactId)
            }
        }
    }

    footer: Item {
        width: width
        height: Theme.spacing
    }
}
