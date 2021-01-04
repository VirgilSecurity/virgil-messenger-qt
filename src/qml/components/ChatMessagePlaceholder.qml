import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

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
        y: firstInRow ? stdTopMargin : 0
        spacing: contentSpacing

        Item { // avatar placeholder. visible on left side
            width: avatarSize
            height: width
            visible: isMessageAlignedLeft
            Avatar {
                id: avatar
                nickname: chatMessage.nickname
                anchors.fill: parent
                visible: firstInRow ? 1 : 0
            }
        }

        Column {
            spacing: stdSmallMargin

            Row { // nickname + time
                id: nicknameTimeRow
                visible: firstInRow
                spacing: 6

                Label {
                    id: nicknameLabel
                    text: isMessageAlignedLeft ? nickname : qsTr("you")
                    color: Theme.labelColor
                    font.pixelSize: UiHelper.fixFontSz(14)
                    anchors.verticalCenter: parent.verticalCenter
                }

                Label {
                    text: displayTime
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

                Rectangle {
                    id: paintingRec
                    visible: !d.isPicture
                    anchors.fill: parent
                    color: d.background
                    radius: stdRadiusHeight

                    transform: Scale {
                        origin.x: paintingRec.width / 2
                        origin.y: paintingRec.height / 2
                        xScale: isMessageAlignedLeft ? 1 : -1
                    }

                    Shape {
                        id: messageShape
                        anchors.fill: parent
                        layer.enabled: true
                        layer.samples: 4

                        ShapePath {
                            strokeColor: "transparent"
                            strokeWidth: 0
                            fillColor: d.background
                            capStyle: ShapePath.RoundCap
                            joinStyle: ShapePath.RoundJoin

                             startX: 0
                             startY: messageShape.height * 0.5
                             PathLine {x: 0; y: 4}

                             PathQuad {x: 4; y: 0; controlX: 0; controlY: 0}

                             PathLine {x: messageShape.width * 0.5; y: 0}

                             PathLine {x: messageShape.width * 0.5; y: messageShape.height}

                             PathLine {x: leftBottomRadiusHeight; y: messageShape.height}

                             PathQuad {x: 0; y: messageShape.height - leftBottomRadiusHeight; controlX: 0; controlY: messageShape.height}
                        }
                    }
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
                            if (attachmentFileExists) {
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
