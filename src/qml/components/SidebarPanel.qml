import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Control {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    readonly property bool isCloudFileList: app.stateManager.currentState === app.stateManager.cloudFileListState
    readonly property int defaultMargin: 9

    Rectangle {
        x: 0
        y: button.y - defaultMargin
        width: parent.width
        height: button.height + 2 * defaultMargin
        color: Theme.contactsBackgroundColor

        readonly property var button: isCloudFileList ? cloudFileListButton : chatListButton
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 2 * defaultMargin

        ImageButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: defaultMargin
            image: "Menu"

            onClicked: contextMenu.open()

            ContextMenu {
                id: contextMenu
                dropdown: true

                Action {
                    text: qsTr("Settings")
                    onTriggered: controllers.users.requestAccountSettings(controllers.users.currentUsername)
                }

                ContextMenuSeparator {
                }

                Action {
                    text: qsTr("Sign Out")
                    onTriggered: controllers.users.signOut()
                }
            }
        }

        SidebarButton {
            id: chatListButton
            Layout.alignment: Qt.AlignHCenter
            onClicked: app.stateManager.openChatList()
        }

        SidebarButton {
            id: cloudFileListButton
            Layout.alignment: Qt.AlignHCenter
            imageSize: 32
            imageSource: "../resources/icons/File-Manager.png"
            onClicked: app.stateManager.openCloudFileList()
        }

        ImageButton {
            Layout.alignment: Qt.AlignHCenter
            image: "Plus"
        }

        Item {
            Layout.fillHeight: true
        }
    }
}

