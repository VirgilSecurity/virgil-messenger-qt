import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15


import "../theme"

Item {
    id: root
    anchors.fill: parent

    readonly property real minInfoHeight: 40
    property real maxInfoTopMargin: height

    QtObject {
        id: d

        readonly property var model: models.cloudFilesProgress
        readonly property real defaultChatHeight: 60
    }

    state: "minInfo"
    states: [
        State {
            name: "minInfo"
            PropertyChanges {
                target: root
                maxInfoTopMargin: root.height
            }
        },

        State {
            name: "maxInfo"
            PropertyChanges {
                target: root
                maxInfoTopMargin: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "minInfo"
            to: "maxInfo"
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
        visible: opacity > 0
        opacity: root.state === "minInfo" && listView.count > 0 ? 1 : 0

        Behavior on opacity {
            NumberAnimation { duration: Theme.animationDuration; easing.type: Easing.OutCubic }
        }

        Behavior on width {
            NumberAnimation { duration: Theme.shortAnimationDuration; easing.type: Easing.OutCubic }
        }

        Label {
            id: shortInfoLabel
            anchors.centerIn: parent
            elide: Label.ElideRight
            text: qsTr("Transfering %1 file(s)").arg(listView.count)
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
                root.state = "maxInfo"
            }
        }
    }

    Page {
        id: maxInfoRec

        anchors {
            fill: parent
            topMargin: root.maxInfoTopMargin
        }
        background: Rectangle {
            color: Theme.contactsBackgroundColor
        }

        header: ToolBar {
            background: HeaderBackground {
            }

            Item {
                id: contentRow
                anchors {
                    fill: parent
                    leftMargin: Theme.margin
                    rightMargin: Theme.smallMargin
                }
                height: 40

                Item {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    height: parent.height

                    HeaderTitle {
                        title: qsTr("Transfer")
                        description: qsTr("Transfering %1 file(s)").arg(listView.count)
                    }

                    ImageButton {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                        }
                        image: "Close"
                        onClicked: root.state = "minInfo"
                    }
                }
            }
        }

        ModelListView {
            id: listView
            anchors {
                fill: parent
                leftMargin: Theme.smallMargin
                rightMargin: Theme.smallMargin
            }
            emptyIcon: "../resources/icons/File-Big.png"
            emptyText: qsTr("Transfered files<br/>will appear here")
            model: d.model
            delegate: listDelegate
        }

        Component {
            id: listDelegate

            ListDelegate {
                id: contactListDelegate
                width: listView.width
                height: d.defaultChatHeight

                Row {
                    Layout.preferredHeight: parent.height
                    Layout.fillWidth: true
                    spacing: Theme.smallSpacing

                    Image {
                        id: multiselectAvatarItem
                        anchors.verticalCenter: parent.verticalCenter
                        height: Theme.avatarHeight
                        source: "../resources/icons/File-Big.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - multiselectAvatarItem.width - parent.spacing
                        spacing: 2

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: model.name
                        }

                        ProgressBar {
                            id: progressBar
                            width: parent.width
                            height: 2
                            from: 0
                            to: model.bytesTotal
                            value: model.bytesLoaded

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
                            font.pointSize: UiHelper.fixFontSz(12)
                            text: model.displayProgress
                            width: parent.width
                            elide: Text.ElideRight
                            textFormat: Text.RichText
                        }
                    }
                }
            }
        }
    }
}
