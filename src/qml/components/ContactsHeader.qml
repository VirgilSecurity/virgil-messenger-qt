import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

ToolBar {
    id: toolbarId
    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias showSeporator: seporator.visible

    default property alias menu: contextMenu.contentData

    property bool isSearchOpen: false

    background: Rectangle {
        implicitHeight: 60
        color: "transparent"

        Rectangle {
            id: seporator
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            height: 1
            color: Theme.chatBackgroundColor
            anchors.bottom: parent.bottom
        }
    }

    RowLayout {
        anchors.fill: parent

        Column {
            Layout.fillWidth: containerId.state === 'closed'
            Layout.leftMargin: 20
            visible: containerId.state === 'closed'

            Label {
                id: titleLabel
                elide: Label.ElideRight

                font.family: Theme.mainFont
                font.pointSize: UiHelper.fixFontSz(15)
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                font.bold: true
                color: Theme.primaryTextColor
            }

            Label {
                id: descriptionLabel
                elide: Label.ElideRight

                font.family: Theme.mainFont
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                font.pointSize: UiHelper.fixFontSz(11)
                color: Theme.secondaryTextColor
            }
        }


        Item {
            id: containerId
            Layout.fillHeight: true
            Layout.preferredWidth: 48
            Layout.fillWidth: containerId.state === 'open'

            state: 'closed'
            states: [
                State {
                    name: "open"

                    ParentChange {
                        target: searchButtonId
                        parent: searchField
                    }

                    PropertyChanges {
                        target: backgroundId
                        color: Theme.inputBackgroundColor
                    }

                    PropertyChanges {
                        target: searchButtonId
                        anchors {
                            left: searchField.left
                            leftMargin: 11
                        }
                    }
                },
                State {
                    name: 'closed'

                    PropertyChanges {
                        target: closeButtonId
                        visible: false
                    }

                    PropertyChanges {
                        target: searchField
                        visible: false
                        text: ''
                    }
                }
            ]

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 100 }
            }

            TextField {
                id: searchField

                anchors.fill: parent;

                leftPadding: 38
                rightPadding: 30

                text: ''

                background: Rectangle {
                    id: backgroundId
                    radius: 20
                    color: "transparent"
                }

                ImageButton {
                    id: closeButtonId
                    imageSource: "../resources/icons/Close.png"
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: parent.right
                        rightMargin: 6
                    }
                    onClicked: {
                        containerId.state = 'closed'
                    }
                }
            }

            ImageButton {
                id: searchButtonId
                imageSource: "../resources/icons/Search.png"
                anchors.centerIn: containerId;
                onClicked: {
                    containerId.state = "open"
                }
            }



        }

        ImageButton {
            Layout.leftMargin: 5
            Layout.rightMargin: 12

            id: menuButton
            imageSource: "../resources/icons/More.png"
            // visible: menu.length
            opacity: menu.length ? 1 : 0
            enabled: menu.length
            onClicked: {
                contextMenu.open()
            }

            ContextMenu {
                id: contextMenu
            }
        }
    }
}
