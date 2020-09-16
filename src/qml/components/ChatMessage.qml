import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"


Control {
    id: chatMessage
    height: row.implicitHeight
    width: loader.item.width

    property string body
    property string time: ""
    property alias nickname: avatar.nickname
    property bool isUser: false
    property string status: ""
    property bool failed: false
    property string messageId

    property bool inRow: false
    property bool firstInRow: true

    property string attachmentId: ""
    property int attachmentBytesTotal: 0
    property string attachmentDisplaySize: ""
    property var attachmentType: undefined
    property string attachmentFilePath: ""
    property string attachmentThumbnailPath: ""
    property int attachmentThumbnailWidth: 0
    property int attachmentThumbnailHeight: 0
    property int attachmentBytesLoaded: 0
    property int attachmentStatus: 0
    property bool attachmentDownloaded: false

    signal saveAttachmentAs(string messageId)

    QtObject {
        id: d
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: isUser ? "#59717D" : Theme.mainBackgroundColor
        readonly property double maxWidth: chatPage.width - 180
        readonly property bool isPicture: hasAttachment && attachmentType == Enums.AttachmentType.Picture
        readonly property double defaultRadius: 4
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
            width: Math.min(implicitWidth, d.maxWidth)
            color: Theme.primaryTextColor
            font.pointSize: UiHelper.fixFontSz(15)
            wrapMode: Text.Wrap
            readOnly: true
            text: chatMessage.body.split("\n").join("<br/>")
            visible: !d.hasAttachment

            property var contextMenu: ContextMenu {
                compact: true

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
                    color: d.isPicture ? "white" : "transparent"

                    Image {
                        id: image
                        Binding on width {
                            when: d.isPicture
                            value: Math.min(3 * attachmentThumbnailWidth, d.maxWidth - 2 * offset)
                        }
                        Binding on height {
                            when: d.isPicture
                            value: width * attachmentThumbnailHeight / attachmentThumbnailWidth
                        }
                        autoTransform: true
                        visible: d.isPicture ? true : attachmentDownloaded && !progressBar.visible
                        source: {
                            if (d.isPicture) {
                                return chatMessage.attachmentDownloaded  ? attachmentFilePath : attachmentThumbnailPath;
                            } else {
                                return "../resources/icons/File Selected Big.png"
                            }
                        }
                    }

                    Rectangle {
                        visible: !attachmentDownloaded && !progressBar.visible
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
                        maxValue: chatMessage.attachmentBytesTotal
                        value: chatMessage.attachmentBytesLoaded
                    }
                }

                ColumnLayout {
                    id: column
                    spacing: 4
                    visible: !d.isPicture
                    readonly property double maxWidth: d.maxWidth - imageRect.width - row.spacing - 2 * offset

                    Label {
                        Layout.fillHeight: true
                        Layout.maximumWidth: column.maxWidth
                        text: chatMessage.body
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(16)
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Layout.maximumWidth: column.maxWidth
                        text: attachmentDisplaySize
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(10)
                    }
                }
            }

            property var contextMenu: ContextMenu {
                compact: true

                Action {
                    text: Platform.isMobile ? qsTr("Save to downloads") : qsTr("Save As...")
                    onTriggered: (Platform.isMobile ? Messenger.downloadAttachment : chatMessage.saveAttachmentAs)(messageId)
                }
            }
        }
    }

    Row {
        id: row
        spacing: 12

        Avatar {
            id: avatar
            width: 30
            opacity: firstInRow ? 1 : 0
            diameter: 30
        }

        Column {
            spacing: 4

            // Nickname + timestamp
            RowLayout {
                visible: firstInRow
                spacing: 6

                Label {
                    text: nickname
                    height: 16
                    color: Theme.labelColor
                    font.pixelSize: UiHelper.fixFontSz(16)
                }

                Label {
                    Layout.alignment: Qt.AlignBottom
                    text: "•  %1".arg(time)
                    color: Theme.labelColor

                    font.pixelSize: UiHelper.fixFontSz(11)
                }
            }

            // Message or attachment
            Rectangle {
                width: loader.item.width
                height: loader.item.height
                color: "transparent"

                Rectangle {
                    width: parent.width
                    height: parent.height
                    color: d.background
                    radius: d.isPicture ? d.defaultRadius : 20
                }

                Rectangle {
                    anchors.top: parent.top
                    height: 22
                    width: 22
                    radius: d.defaultRadius
                    color: d.background
                }

                Rectangle {
                    visible: inRow
                    anchors.bottom: parent.bottom
                    height: 22
                    width: 22
                    radius: d.defaultRadius
                    color: d.background
                }

                Loader {
                    id: loader
                    sourceComponent: d.hasAttachment ? attachmentComponent : textEditComponent

                    property var contextMenu: item.contextMenu
                    function openContextMenu(mouse) {
                        contextMenu.x = mouse.x
                        contextMenu.y = mouse.y
                        contextMenu.open()
                    }

                    function downloadOpen(messageId) {
                        (d.isPicture ? Messenger.openAttachment : Messenger.downloadAttachment)(messageId)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Platform.isDesktop ? (Qt.LeftButton | Qt.RightButton) : Qt.LeftButton

                    onClicked: function(mouse) {
                        if (Platform.isDesktop && mouse.button == Qt.RightButton) {
                            loader.openContextMenu(mouse)
                        }
                        else {
                            loader.downloadOpen(messageId)
                        }
                    }
                    onPressAndHold: {
                        if (Platform.isMobile) {
                            loader.openContextMenu(mouse)
                        }
                    }
                }
            }

            // Status label
            Label {
                id: statusLabel
                height: 12
                text: {
                    if (chatMessage.failed) {
                        return "failed"
                    }
                    switch (status) {
                        case "0": return "sending"
                        case "1": return "sent"
                        case "2": return "delivered"
                        case "4": return "sending" // attachment
                        default: return ""
                    }
                }
                color: chatMessage.failed ? "red" : Theme.labelColor
                font.pixelSize: UiHelper.fixFontSz(11)
            }
        }
    }
}
