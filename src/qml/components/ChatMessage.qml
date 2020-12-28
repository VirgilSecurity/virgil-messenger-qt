import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

import "../base"
import "../theme"

Control {
    id: chatMessage
    height: messageLoader.height

    property int thisIndex: -1
    property var thisDay

    property double maxWidth: 0
    readonly property real leftIndent: isMessageAlignedLeft ? avatarSize + contentSpacing + Theme.margin : avatarSize + contentSpacing

    property string body: ""
    property string displayTime: ""
    property string nickname
    property bool isOwnMessage: false
    property string status: ""

    property bool isBroken: false
    property string messageId: ""
    property bool inRow: false
    property bool firstInRow: true

    property string attachmentId: ""
    property bool attachmentTypeIsFile: false
    property bool attachmentTypeIsPicture: false
    property bool attachmentIsLoading: false
    property bool attachmentIsLoaded: false
    property int attachmentBytesTotal: 0
    property string attachmentDisplaySize: ""
    property string attachmentDisplayText: ""
    property string attachmentIconPath: ""
    property int attachmentPictureThumbnailWidth: 0
    property int attachmentPictureThumbnailHeight: 0
    property string attachmentDisplayProgress: ""
    property int attachmentBytesLoaded: 0
    property bool attachmentFileExists: false

    signal saveAttachmentAs(string messageId)
    signal openContextMenu(string messageId, var mouse, var contextMenu)

    readonly property int stdCheckHeight: 14
    readonly property int stdTopMargin: 15
    readonly property int stdSmallMargin: 5
    readonly property int stdRadiusHeight: 20
    readonly property int stdRectangleSize: 22
    property int leftBottomRadiusHeight: inRow ? 4 : stdRadiusHeight

    readonly property int avatarSize: 30
    readonly property int contentSpacing: 12

    readonly property bool isMessageAlignedLeft: {
        if (!Platform.isMobile) {
            if (isOwnMessage) {
                if (chatMessage.width > 500) {
                    return true
                } else {
                    return false
                }
            } else {
                return true
            }
        }

        if (Platform.isMobile) {
            if (isOwnMessage) {
                return false
            } else {
                return true
            }
        }
    }


    Behavior on leftBottomRadiusHeight {
        NumberAnimation { duration: Theme.animationDuration}
    }

    QtObject {
        id: d
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: isOwnMessage ? "#59717D" : Theme.mainBackgroundColor
        readonly property bool isPicture: attachmentTypeIsPicture
        readonly property double defaultRadius: 4
    }

    Loader {
        id: messageLoader
        width: parent.width
        sourceComponent: smartMessage
    }

    Component {
        id: smartMessage

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

                        Item { // status
                            width: 14
                            height: width
                            anchors{
                                left: parent.right
                                leftMargin: -width * 0.7
                                bottom: parent.bottom
                            }
                            visible: isOwnMessage
                            Image {
                                width: parent.width
                                anchors.bottom: parent.bottom
                                fillMode: Image.PreserveAspectFit
                                source: "../resources/icons/%1.png".arg(messageStatusImageSource())
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
        }
    }

    Component {
        id: textEditComponent

        TextEdit {
            id: textEdit
            property var contextMenu: ContextMenu {
                compact: true
                enabled: !chatMessage.isBroken

                Action {
                    text: qsTr("Copy")
                    onTriggered: clipboard.setText(textEdit.getText(0, textEdit.length))
                }
            }

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
                    color: d.isPicture && attachmentIconPath ? "white" : "transparent"

                    Image {
                        id: image
                        Binding on width {
                            when: d.isPicture
                            value: image.pictureWidth
                        }
                        Binding on height {
                            when: d.isPicture
                            value: image.pictureWidth * attachmentPictureThumbnailHeight / attachmentPictureThumbnailWidth
                        }
                        autoTransform: true
                        visible: d.isPicture ? true : attachmentFileExists && !progressBar.visible
                        source: chatMessage.attachmentIconPath

                        readonly property double pictureWidth: d.isPicture ? Math.min(3 * attachmentPictureThumbnailWidth, maxWidth - 2 * offset) : 0
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
                        visible: chatMessage.attachmentIsLoading
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
                        visible: chatMessage.attachmentIsLoaded
                        text: attachmentDisplaySize
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(10)
                    }

                    Label {
                        Layout.maximumWidth: column.maxWidth
                        visible: chatMessage.attachmentIsLoading
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

//  Smart items
    Item {
        id: newMessageItem
        property bool isVisible: messageId === idOfFirstUnreadMessage
        onIsVisibleChanged: {
            if (isVisible) {
                newMessageItem.opacity = 1
            } else {
                opacityOffDelay.restart()
            }
        }

        anchors {
            left: parent.left
            leftMargin: leftIndent
            right: parent.right
            top: parent.top
            topMargin: firstInRow ? contentSpacing : -contentSpacing / 4
        }

        height: 1
        opacity: 0
        visible: newMessageItem.opacity > 0

        Timer {
            id: opacityOffDelay
            running: false
            repeat: false
            interval: 1500
            onTriggered: newMessageItem.opacity = 0
        }

        Behavior on opacity {
            NumberAnimation { duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
        }

        Item {
            width: parent.width
            height: unreadMessageRec.height
            anchors.verticalCenter: parent.verticalCenter
            layer.enabled: true

            Rectangle {
                width: parent.width
                height: 1
                color: Theme.buttonPrimaryColor
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                id: unreadMessageRec
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }

                height: newMessage.height + stdSmallMargin
                width: newMessage.width + stdSmallMargin
                radius: 3
                color: Theme.buttonPrimaryColor

                Rectangle {
                    anchors {
                        horizontalCenter: parent.left
                        horizontalCenterOffset: 2
                        verticalCenter: parent.verticalCenter
                    }

                    height: parent.height * 0.65
                    width: height
                    rotation: 45
                    color: parent.color
                }

                Label {
                    id: newMessage
                    text: qsTr("NEW")
                    color: Theme.primaryTextColor
                    font.pixelSize: UiHelper.fixFontSz(8)
                    font.bold: true
                    anchors.centerIn: parent
                }
            }
        }
    }

    function messageStatusImageSource() {
        if (chatMessage.isBroken) {
            return "M-Sending" // "broken"
        }
        switch (status) {
            case "0": return "M-Sending" // "sending"
            case "1": return "M-Sent" // "sent"
            case "2": return "M-Delivered" // "delivered"
            case "4": return "M-Read" // "read"
            default: return "M-Read" // "read"
        }
    }
}
