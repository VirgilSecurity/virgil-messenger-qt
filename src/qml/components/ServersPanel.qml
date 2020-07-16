import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

Control {
    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    default property alias menu: contextMenu.contentData

    ColumnLayout {

        anchors.fill: parent

        ImageButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
            Layout.bottomMargin: 9
            image: "Menu"

            onClicked: {
                contextMenu.open()
            }

            ContextMenu {
                id: contextMenu
                dropdown: true
            }
        }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            implicitHeight: 36
            implicitWidth: 36
            color: "white"
            radius: 9

            Image {
                anchors.centerIn: parent
                width: 28
                height: 28
                source: "../resources/icons/Logo.png"
            }
        }

        ImageButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
            Layout.bottomMargin: 9
            image: "Plus"
        }

        Item {
            Layout.fillHeight: true
        }
    }
}

