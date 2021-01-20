import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15


import "../theme"

Item {
    id: fileManagerUploadDownload
    anchors.fill: parent

    readonly property real minInfoHeight: 40
    property real maxInfoTopMargin: height

    state: "minInfo"
    states: [
        State {
            name: "minInfo"
            PropertyChanges {
                target: minInfoRec
                opacity: 1
            }
            PropertyChanges {
                target: fileManagerUploadDownload
                maxInfoTopMargin: fileManagerUploadDownload.height
            }
        },

        State {
            name: "maxInfo"
            PropertyChanges {
                target: minInfoRec
                opacity: 0
            }
            PropertyChanges {
                target: fileManagerUploadDownload
                maxInfoTopMargin: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "minInfo"
            to: "maxInfo"
            NumberAnimation {
                property: 'opacity'
                duration: Theme.shortAnimationDuration
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                property: 'maxInfoTopMargin'
                duration: Theme.longAnimationDuration
                easing.type: Easing.OutExpo
            }
        },

        Transition {
            from: "maxInfo"
            to: "minInfo"
            NumberAnimation {
                property: 'opacity'
                duration: Theme.animationDuration
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                property: 'maxInfoTopMargin'
                duration: Theme.longAnimationDuration
                easing.type: Easing.OutExpo
            }
        }
    ]

    Rectangle {
        id: minInfoRec
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: minInfoHeight
        }

        width: shortInfoLabel.width + Theme.margin * 2
        height: minInfoHeight
        radius: height * 0.5
        color: Theme.chatSeparatorColor

        Behavior on width {
            NumberAnimation { duration: Theme.shortAnimationDuration; easing.type: Easing.OutCubic }
        }

        Label {
            id: shortInfoLabel
            anchors.centerIn: parent
            elide: Label.ElideRight
            text: qsTr("Uploading file 7 of 9")
            font.family: Theme.mainFont
            font.pointSize: UiHelper.fixFontSz(12)
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            color: Theme.primaryTextColor
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                cursorShape = Qt.PointingHandCursor
                minInfoRec.scale = 1.02
            }

            onExited:  {
                cursorShape = Qt.ArrowCursor
                minInfoRec.scale = 1
            }

            onClicked: {
                fileManagerUploadDownload.state = "maxInfo"
            }
        }
    }

    Item {
        id: maxInfoRec
        anchors {
            fill: parent
            topMargin: fileManagerUploadDownload.maxInfoTopMargin
        }

        Page {
            anchors.fill: parent
            background: Rectangle {
                color: Theme.chatBackgroundColor
            }

            header: Control {
                id: headerControl
                width: parent.width
                height: Theme.headerHeight
                z: 1

                background: Rectangle {
                    color: Theme.chatBackgroundColor
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5

                    HorizontalRule {
                        anchors.bottom: parent.bottom
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        color: Theme.chatSeparatorColor
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.AllButtons
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.smallMargin
                    anchors.rightMargin: Theme.smallMargin

                    ImageButton {
                        image: "Arrow-Left"
                        onClicked: fileManagerUploadDownload.state = "minInfo"
                    }

                    Column {
                        Layout.fillWidth: true
                        Layout.leftMargin: Theme.smallMargin
                        Label {
                            text: qsTr("Progress")
                            font.pointSize: UiHelper.fixFontSz(15)
                            color: Theme.primaryTextColor
                            font.bold: true
                        }
                        Label {
                            topPadding: 2
                            text: qsTr("Uploading file 7 of 9")
                            font.pointSize: UiHelper.fixFontSz(12)
                            color: Theme.secondaryTextColor
                        }
                    }

                    ImageButton {
                        image: "More"
                        visible: true
                        opacity: 0
                    }
                }
            }
        }
    }
}
