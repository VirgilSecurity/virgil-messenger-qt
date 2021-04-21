import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Control {
    id: root

    readonly property alias buttonIndex: d.buttonIndex

    signal chatListRequested()
    signal cloudFileListRequested()
    signal accountSettingsRequested()

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    QtObject {
        id: d
        property int buttonIndex: 0 // chat list
        readonly property var button: buttonIndex === 0 ? chatListButton : cloudFileListButton

        Component.onCompleted: controllers.cloudFiles.permissionsChecked.connect(root.cloudFileListRequested)
    }

    Rectangle {
        x: 0
        y: d.button.y - Theme.smallMargin
        width: parent.width
        height: d.button.height + 2 * Theme.smallMargin
        color: Theme.contactsBackgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 2 * Theme.smallMargin

        ImageButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Theme.smallMargin
            image: "Menu"

            onClicked: contextMenu.open()

            ContextMenu {
                id: contextMenu
                dropdown: true

                Action {
                    text: qsTr("Settings")
                    onTriggered: root.accountSettingsRequested()
                }

                ContextMenuSeparator {
                }

                Action {
                    text: qsTr("Sign Out")
                    onTriggered: messenger.signOut()
                }
            }
        }

        SidebarButton {
            id: chatListButton
            Layout.alignment: Qt.AlignHCenter
            onClicked: root.chatListRequested()
        }

        SidebarButton {
            id: cloudFileListButton
            Layout.alignment: Qt.AlignHCenter
            imageSize: 32
            imageSource: "../resources/icons/File-Manager.png"
            onClicked: controllers.cloudFiles.checkPermissions()
        }

        ImageButton {
            Layout.alignment: Qt.AlignHCenter
            image: "Plus"
        }

        Item {
            Layout.fillHeight: true
        }
    }

    onChatListRequested: d.buttonIndex = 0
    onCloudFileListRequested: d.buttonIndex = 1
}

