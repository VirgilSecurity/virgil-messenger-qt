import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Customers 1.0

import "../theme"

Control {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    readonly property bool isFileCloud: app.stateManager.currentState === manager.fileCloudState
    readonly property int defaultMargin: 9

    default property alias menu: contextMenu.contentData

    Rectangle {
        x: 0
        y: button.y - defaultMargin
        width: parent.width
        height: button.height + 2 * defaultMargin
        color: Theme.contactsBackgroundColor

        readonly property var button: isFileCloud ? fileCloudButton : chatListButton
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
            }
        }

        Rectangle {
            id: chatListButton
            Layout.alignment: Qt.AlignHCenter
            implicitHeight: 36
            implicitWidth: 36
            color: Customer.serverIconBackgroundColor
            radius: defaultMargin

            Image {
                anchors.centerIn: parent
                width: 28
                height: 28
                source: "../resources/icons/Logo.png"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                onClicked: app.stateManager.openChatList()
            }
        }

        ImageButton {
            id: fileCloudButton
            Layout.alignment: Qt.AlignHCenter
            image: "File Selected Big"
            visible: settings.fileCloudEnabled

            onClicked: app.stateManager.openFileCloud()
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

