import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"


Rectangle {
    id: uploadProgressBar

    property var model: models.fileCloudUploader
    property int fileIndex: model.currentIndex
    property int filesCount: model.fileNames.length
    property real fileUploadValue: model.currentProcessedBytes * 100 / model.currentTotalBytes

    property real uploadHeight: 60
    property real uploadOpacity: 1

    property real uploadFrom: 0
    property real uploadTo: 100

    color: Theme.mainBackgroundColor

    Rectangle {
        id: separator
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.chatBackgroundColor
    }

    state: filesCount !== 0 ? "opened" : "closed"
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
                duration: Theme.animationDuration
            }
        }
    ]

    Column {
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.margin
            rightMargin: Theme.margin
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
            value: fileUploadValue

            Behavior on value {
                NumberAnimation { duration: Theme.animationDuration * 2 }
            }

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
            font.pointSize: UiHelper.fixFontSz(10)
            text: fileIndex + "/" + filesCount + " " + model.fileNames[fileIndex]
            width: parent.width
            elide: Text.ElideRight
        }
    }
}
