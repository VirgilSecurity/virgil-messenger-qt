import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.1

import "../../theme"

ToolBar {
    spacing: 20
    height: 60

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    Item {
        id: container
        anchors.leftMargin: 20
        anchors.rightMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 15
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Column {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Server Name")
                        font.pointSize: UiHelper.fixFontSz(15)
                        color: Theme.primaryTextColor
                        font.bold: true
                    }

                    Label {
                        text: qsTr("Server description")
                        font.pointSize: UiHelper.fixFontSz(12)
                        color: Theme.secondaryTextColor
                    }
                }

                ToolButton {
                    id: settingsBtn
                    Layout.alignment: Qt.AlignRight
                    Layout.minimumHeight: 40
                    Layout.minimumWidth: 40
                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        addContact()
                    }

                    Image {
                        source: "qrc:/qml/resources/icons/Search.png"
                        width: 24
                        height: 24
                        smooth: true
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: settingsBtn.horizontalCenter
                    }
                }

                ToolButton {
                    id: control
                    text: qsTr("⋮")

                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {
                        menu.x = control.width - menu.width
                        menu.y = control.height + container.anchors.bottomMargin

                        menu.open()
                    }
                    background: Rectangle {
                        color: "transparent"
                    }

                    contentItem: Text {
                         text: control.text
                         color: Theme.primaryTextColor
                         font.pointSize: UiHelper.fixFontSz(24)
                    }

                    Menu {
                        id: menu

                        property real menuWidth: 200
                        property real menuItemHeight: 40
                        property real menuItemPadding: 20


                        implicitWidth: menuWidth

                        topPadding: 10
                        bottomPadding: 10

                        Action { text: qsTr("New chat") }

                        Action {
                            text: qsTr("Settings")
                            onTriggered: showSettings()

                        }

                        MenuSeparator {
                            leftPadding: menu.menuItemPadding
                            rightPadding: menu.menuItemPadding
                            contentItem: Rectangle {
                                implicitHeight: 1
                                implicitWidth: menu.menuWidth
                                color: Theme.menuSeparatorColor
                            }
                        }

                        Action {
                            text: qsTr("Sign out")
                            onTriggered: logout()

                        }

                        background: Rectangle {
                            implicitWidth: menu.menuWidth
                            implicitHeight: menu.contentHeight
                            color: Theme.menuBackgroundColor
                            radius: 6
                        }

                        delegate: MenuItem {
                            id: menuItem
                            implicitWidth: parent.width
                            implicitHeight: menu.menuItemHeight
                            font.pointSize: UiHelper.fixFontSz(15)

                            contentItem: Text {
                                leftPadding: menu.menuItemPadding
                                rightPadding:menu.menuItemPadding
                                text: menuItem.text
                                font: menuItem.font
                                color: menuItem.highlighted ? Theme.secondaryTextColor : Theme.primaryTextColor
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }


            ToolSeparator {
                id: separator
                orientation: Qt.Horizontal
                Layout.fillWidth: true

                contentItem: Rectangle {
                    implicitHeight: 1
                    width: parent.width
                    color: Theme.chatSeparatorColor
                }
            }
        }
    }


}
