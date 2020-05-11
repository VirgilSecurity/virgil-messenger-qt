import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"
import "../components"

ToolBar {

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias showBackButton: backButton.visible
    property alias showSeporator: seporator.visible
    default property alias menu: contextMenu.contentData

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
            Layout.fillWidth: true
            Layout.leftMargin: 20

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

        ImageButton {

            id: backButton
            imageSource: "../resources/icons/Search.png"

            // TODO: Componets sholdn't know about mainView
            // The logic must be as abastract as possible.
            onClicked: mainView.back()
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

/*

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
            width: parent.width

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Column {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Default")
                        font.pointSize: UiHelper.fixFontSz(15)
                        color: Theme.primaryTextColor
                        font.bold: true
                    }
                    Label {
                        text: qsTr("Server")
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

                    // onClicked: addContact()

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

                    Action {
                        text: qsTr("New chat")
                        onTriggered: addContact()
                    }

                    Action {
                        text: qsTr("Settings")
                        onTriggered: mainView.showAccountSettings()
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
                        onTriggered: mainView.signOut()
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

*/
