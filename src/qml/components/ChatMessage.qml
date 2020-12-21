import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"

Control {
    id: chatMessage
    height: row.height
    width: loader.item.width

    property int thisIndex: -1

    property double maxWidth: parent.width
    readonly property real leftIndent: row.spacing + avatar.width

    property string body: ""
    property string displayTime: ""
    property alias nickname: avatar.nickname
    property bool isOwnMessage: false
    property string status: ""

    property bool isBroken: false
    property string messageId: ""
    property bool inRow: false
    property bool firstInRow: true

    property string attachmentId: ""
    property var attachmentType: undefined
    property var attachmentStatus: undefined
    property int attachmentBytesTotal: 0
    property string attachmentDisplaySize: ""
    property string attachmentDisplayText: ""
    property string attachmentDisplayProgress: ""
    property string attachmentImagePath: ""
    property int attachmentThumbnailWidth: 0
    property int attachmentThumbnailHeight: 0
    property int attachmentBytesLoaded: 0
    property bool attachmentFileExists: false

    signal saveAttachmentAs(string messageId)
    signal openContextMenu(string messageId, var mouse, var contextMenu)

    readonly property int stdCheckHeight: 14
    readonly property int stdRadiusHeight: 20
    readonly property int stdRectangleSize: 22
    property int leftBottomRadiusHeight: inRow ? 4 : stdRadiusHeight

    Behavior on leftBottomRadiusHeight {
        NumberAnimation { duration: Theme.animationDuration}
    }

    QtObject {
        id: d
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: isOwnMessage ? "#59717D" : Theme.mainBackgroundColor
        readonly property bool isPicture: hasAttachment && attachmentType == Enums.AttachmentType.Picture
        readonly property double defaultRadius: 4
    }

    Row {
        id: row
        spacing: 12

        Column {
            anchors.top: parent.top

            Item {
                visible: firstInRow
                width: parent.width
                height: 3
            }

            Avatar {
                id: avatar
                width: 30
                height: width
                opacity: firstInRow ? 1 : 0
            }
        }

        Column {
            spacing: 4
            anchors.top: parent.top

            // Nickname + timestamp

            Item {
                id: nicknameTopSeparator
                visible: firstInRow
                width: parent.width
                height: 5
            }

            Row {
                visible: firstInRow
                spacing: 6

                Label {
                    text: nickname
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
                        verticalCenterOffset: 1
                    }
                }
            }

            // Message or attachment with status
            Item {
                width: loader.item.width
                height: loader.item.height

                Rectangle {
                    visible: d.isPicture
                    width: parent.width
                    height: parent.height
                    color: d.background
                    radius: d.defaultRadius
                }

                Rectangle {
                    id: paintingRec
                    visible: !d.isPicture
                    width: parent.width
                    height: parent.height
                    color: d.background
                    radius: stdRadiusHeight

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

                Item {
                    id: messageStatus
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: parent.right
                        rightMargin: -Theme.margin
                    }
                    width: stdCheckHeight
                    height: stdCheckHeight

                    Image {
                        anchors.fill: parent
                        source: "../resources/icons/Check.png"
                        visible: isCheckVisible()
                    }

                    Item {
                        id: messageIsBroken
                        anchors.fill: parent
                        visible: chatMessage.isBroken
                        Repeater {
                            model: 2
                            Rectangle {
                                anchors.centerIn: parent
                                width: 0.75 * parent.width
                                height: 2
                                radius: height
                                color: Theme.buttonPrimaryColor
                                rotation: index ? -45 : 45
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: textEditComponent

        TextEdit {
            id: textEdit
            topPadding: 12
            bottomPadding: 12
            leftPadding: 15
            rightPadding: 15
            textFormat: Text.RichText
            width: Math.min(implicitWidth, maxWidth)
            color: Theme.primaryTextColor
            font.pointSize: UiHelper.fixFontSz(15)
            wrapMode: Text.Wrap
            readOnly: true
            text: chatMessage.body;
            visible: !d.hasAttachment

            property var contextMenu: ContextMenu {
                compact: true
                enabled: !chatMessage.isBroken

                Action {
                    text: qsTr("Copy")
                    onTriggered: clipboard.setText(textEdit.getText(0, textEdit.length))
                }
            }
        }
    }

    Component {
        id: attachmentComponent

        Item {
            height: row.height + 2 * offset
            width: row.width + 2 * offset

            readonly property double offset: d.isPicture ? d.defaultRadius : 12

            Row {
                id: row
                spacing: 14
                x: offset
                y: offset

                Rectangle {
                    id: imageRect
                    y: 0.5 * (row.height - height)
                    width: image.width
                    height: image.height
                    color: d.isPicture && attachmentImagePath ? "white" : "transparent"

                    Image {
                        id: image
                        Binding on width {
                            when: d.isPicture
                            value: image.pictureWidth
                        }
                        Binding on height {
                            when: d.isPicture
                            value: image.pictureWidth * attachmentThumbnailHeight / attachmentThumbnailWidth
                        }
                        autoTransform: true
                        visible: d.isPicture ? true : attachmentFileExists && !progressBar.visible
                        source: chatMessage.attachmentImagePath

                        readonly property double pictureWidth: d.isPicture ? Math.min(3 * attachmentThumbnailWidth, maxWidth - 2 * offset) : 0
                    }

                    Rectangle {
                        visible: !attachmentFileExists && !progressBar.visible
                        anchors.centerIn: parent
                        width: 1.3333 * downloadImage.width
                        height: width
                        radius: 0.5 * width
                        color: "#364542"

                        Image {
                            id: downloadImage
                            anchors.centerIn: parent
                            source: "../resources/icons/File Download Big.png"
                        }
                    }

                    CircleProgressBar {
                        id: progressBar
                        anchors.centerIn: parent
                        size: 40
                        visible: chatMessage.attachmentStatus == Enums.AttachmentStatus.Loading
                        maxValue: Math.max(1, chatMessage.attachmentBytesTotal)
                        value: Math.min(0.99 * maxValue, Math.max(0.01 * maxValue, chatMessage.attachmentBytesLoaded))
                        animated: visible
                    }
                }

                ColumnLayout {
                    id: column
                    spacing: 4
                    visible: !d.isPicture
                    readonly property double maxWidth: chatMessage.maxWidth - imageRect.width - row.spacing - 2 * offset

                    Label {
                        Layout.fillHeight: true
                        Layout.maximumWidth: column.maxWidth
                        text: chatMessage.attachmentDisplayText
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(16)
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Layout.maximumWidth: column.maxWidth
                        visible: chatMessage.attachmentStatus == Enums.AttachmentStatus.Loaded
                        text: attachmentDisplaySize
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(10)
                    }

                    Label {
                        Layout.maximumWidth: column.maxWidth
                        visible: chatMessage.attachmentStatus == Enums.AttachmentStatus.Loading
                        text: attachmentDisplayProgress
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(10)
                    }
                }
            }

            property var contextMenu: ContextMenu {
                compact: true
                enabled: !chatMessage.isBroken

                Action {
                    text: Platform.isMobile ? qsTr("Save to downloads") : qsTr("Save As...")
                    onTriggered: (Platform.isMobile ? controllers.attachments.download : chatMessage.saveAttachmentAs)(messageId)
                }
            }
        }
    }

    function isCheckVisible() {
        if (chatMessage.isBroken) {
            return false
        }
        switch (status) {
            case "0": return false // "sending"
            case "1": return true // "sent"
            case "2": return true // "delivered"
            case "4": return false // "sending"
            default: return false
        }
    }

    Component.onCompleted: {
//        console.log("->", messageId, body, displayTime, nickname, status, isBroken)
//        console.log(attachmentId, attachmentType, attachmentStatus, attachmentImagePath, attachmentFileExists)
    }
}
