import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../base"
import "../theme"

Item {
    height: contentRow.height + contentRow.y
    width: parent.width
    layer.enabled: true
    state: isMessageAlignedLeft ? "aligned left" : "aligned right"
    states: [
        State {
            name: "aligned left"

            AnchorChanges {
                target: contentRow
                anchors.left: parent.left
                anchors.right: undefined
            }
            AnchorChanges {
                target: nicknameTimeRow
                anchors.left: parent.left
                anchors.right: undefined
            }
            AnchorChanges {
                target: messageContentItem
                anchors.left: parent.left
                anchors.right: undefined
            }
        },
        State {
            name: "aligned right"

            AnchorChanges {
                target: contentRow
                anchors.left: undefined
                anchors.right: parent.right
            }
            AnchorChanges {
                target: nicknameTimeRow
                anchors.left: undefined
                anchors.right: parent.right
            }
            AnchorChanges {
                target: messageContentItem
                anchors.left: undefined
                anchors.right: parent.right
            }
        }
    ]

    Row {
        id: contentRow
        spacing: contentSpacing

        Item { // avatar placeholder. visible on left side
            width: avatarSize
            height: width
            visible: isMessageAlignedLeft
            Avatar {
                id: avatar
                nickname: model.senderUsername
                anchors.fill: parent
                visible: model.firstInRow ? 1 : 0
            }
        }

        Column {
            spacing: stdSmallMargin

            Row { // nickname + time
                id: nicknameTimeRow
                visible: model.firstInRow
                spacing: 6

                Label {
                    id: nicknameLabel
                    text: isMessageAlignedLeft ? model.senderUsername : qsTr("you")
                    color: Theme.labelColor
                    font.pixelSize: UiHelper.fixFontSz(14)
                    anchors.verticalCenter: parent.verticalCenter
                }

                Label {
                    text: model.displayTime
                    color: Theme.labelColor
                    font.pixelSize: UiHelper.fixFontSz(11)
                    anchors {
                        verticalCenter: parent.verticalCenter
                    }
                }
            }

            Item { // message content
                id: messageContentItem
                width: loader.item.width
                height: loader.item.height

                Rectangle {
                    visible: d.isPicture
                    anchors.fill: parent
                    color: d.background
                    radius: d.defaultRadius
                }

                MessageShape {
                    id: messageShape
                    anchors.fill: parent
                }

                // status icon
                Item {
                    width: 24
                    height: width
                    anchors{
                        left: parent.right
                        leftMargin: d.isPicture ? 1 : -3
                        bottom: parent.bottom
                    }
                    visible: isOwnMessage

                    Image {
                        width: parent.width
                        anchors.bottom: parent.bottom
                        fillMode: Image.PreserveAspectFit
                        source: statusIcon
                    }
                }

                Loader {
                    id: loader
                    sourceComponent: d.hasAttachment ? attachmentComponent : textEditComponent
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Platform.isDesktop ? (Qt.LeftButton | Qt.RightButton) : Qt.LeftButton

                    onClicked: function(mouse) {
                        if (Platform.isDesktop && mouse.button == Qt.RightButton) {
                            var coord = mapToItem(chatMessage, mouse.x, mouse.y)
                            openContextMenu(messageId, coord, loader.item.contextMenu)
                        }
                        else if (d.hasAttachment) {
                            if (model.attachmentFileExists) {
                                controllers.attachments.open(messageId)
                            }
                            else {
                                controllers.attachments.download(messageId)
                            }
                        }
                    }
                    onPressAndHold: {
                        if (Platform.isMobile) {
                            var coord = mapToItem(chatMessage, mouse.x, mouse.y)
                            openContextMenu(messageId, coord, loader.item.contextMenu)
                        }
                    }
                }
            }
        }

        Item { // right separator for right-aligned messages
            width: Theme.smallMargin
            height: 1
            visible: !isMessageAlignedLeft
        }
    }

    Component {
        id: textEditComponent
        MessageContentText {}
    }

    Component {
        id: attachmentComponent
        MessageContentAttachment {}
    }
}
