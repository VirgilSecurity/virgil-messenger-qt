import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../../theme"
import "../Buttons"

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

                    onClicked: addContact()

                    Image {
                        source: "../../resources/icons/Search.png"
                        width: 24
                        height: 24
                        smooth: true
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: settingsBtn.horizontalCenter
                    }
                }

                SettingsButton {
                    id: menu
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.fillHeight: true

                    Action { text: qsTr("New chat") }

                    Action {
                        text: qsTr("Settings")
                        onTriggered: mainLayout.showUserSettings()
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
