import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"


Item {
    id: uploadProgressBar

    property real uploadHeight: 60
    property real uploadOpacity: 1

    property real uploadFrom: 0
    property real uploadTo: 100
    property real uploadValue: 0

    property var fileIndex: 1
    property var filesCount: 5
    property var fileName: "Test_file.jpg"


    state: "closed"
    states: [
        State {
            name: "opened"

            PropertyChanges {
                target: uploadProgressBar
                opacity: uploadOpacity
                height: uploadHeight
            }
        },
        State {
            name: "closed"

            PropertyChanges {
                target: uploadProgressBar
                opacity: 0
                height: 0
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties: "opacity, height"
                easing.type: Easing.InExpo
                duration: 250
            }
        }
    ]

    Column {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        height: this.contentHeight
        spacing: 10

        ProgressBar {
            id: progressBar
            width: parent.width
            height: 6
            from: uploadFrom
            to: uploadTo
            value: uploadValue

            background: Rectangle {
                implicitWidth: progressBar.width
                implicitHeight: progressBar.height
                radius: height
                color: Theme.menuSeparatorColor
            }

            contentItem: Item {
                implicitWidth: progressBar.width
                implicitHeight: progressBar.height

                Rectangle {
                    width: parent.width * progressBar.visualPosition
                    height: parent.height
                    radius: height
                    color: Theme.buttonPrimaryColor
                }
            }
        }

        Text {
            color: Theme.secondaryTextColor
            font.pointSize: UiHelper.fixFontSz(12)
            text: fileIndex + "/" + filesCount + " " + fileName
            width: parent.width
            elide: Text.ElideRight
        }
    }
}
