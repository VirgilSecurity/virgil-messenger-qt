import QtQuick 2.12
import QtQuick.Controls 2.15

import "../theme"

Rectangle {
    id: uploadProgressBar

    readonly property var model: models.fileCloudUploader
    readonly property int fileIndex: model.currentIndex
    readonly property int filesCount: model.fileNames.length
    readonly property real fileUploadValue: model.currentProcessedBytes

    readonly property real uploadHeight: 60
    readonly property real uploadOpacity: 1

    readonly property real uploadFrom: 0
    readonly property real uploadTo: model.currentTotalBytes

    color: Theme.mainBackgroundColor

    HorizontalRule {
        id: separator
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
        spacing: Theme.smallSpacing

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
