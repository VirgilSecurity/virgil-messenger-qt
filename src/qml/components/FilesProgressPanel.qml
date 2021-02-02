import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "../theme"

Item {
    id: root
    anchors.fill: parent

    property bool buttonVisible: true

    QtObject {
        id: d

        property real maxInfoTopMargin: root.height
        readonly property var model: models.cloudFilesProgress
        readonly property real defaultChatHeight: 60
    }

    state: "minInfo"
    states: [
        State {
            name: "minInfo"
            PropertyChanges {
                target: d
                maxInfoTopMargin: root.height
            }
        },

        State {
            name: "maxInfo"
            PropertyChanges {
                target: d
                maxInfoTopMargin: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "minInfo"
            to: "maxInfo"
            NumberAnimation {
                target: d
                property: "maxInfoTopMargin"
                duration: Theme.longAnimationDuration
                easing.type: Easing.OutExpo
            }
        },

        Transition {
            from: "maxInfo"
            to: "minInfo"
            NumberAnimation {
                target: d
                property: "maxInfoTopMargin"
                duration: Theme.longAnimationDuration
                easing.type: Easing.OutExpo
            }
        }
    ]

    ListStatusButton {
        visible: buttonVisible && root.state === "minInfo" && listView.count > 0
        text: qsTr("Transfering %1 file(s)").arg(listView.count)

        onClicked: {
            root.state = "maxInfo"
        }
    }

    Page {
        id: maxInfoRec

        anchors {
            fill: parent
            topMargin: d.maxInfoTopMargin
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

                    // TODO(fpohtmeh): create component?
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
                        width: parent.width - multiselectAvatarItem.width - closeButton.width - 2 * parent.spacing
                        spacing: 2

                        Text {
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: model.name
                        }

                        LineProgressBar {
                            width: parent.width
                            to: model.bytesTotal
                            value: model.bytesLoaded
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

                    ImageButton {
                        id: closeButton
                        anchors.verticalCenter: parent.verticalCenter
                        image: "Close"
                        onClicked: d.model.interrupt(model.id)
                    }
                }
            }
        }
    }
}
