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

    property bool inRow: false
    property bool firstInRow: true

    property string attachmentId
    property string attachmentBytesTotal
    property string attachmentDisplaySize
    property var attachmentType
    property string attachmentLocalUrl
    property string attachmentThumbnailUrl
    property int attachmentBytesLoaded
    property int attachmentStatus
    property bool attachmentDownloaded

    signal saveAttachmentAs()

    QtObject {
        id: d
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: isUser ? "#59717D" : Theme.mainBackgroundColor
        readonly property double maxWidth: chatPage.width - 40
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
            width: Math.min(implicitWidth,  d.maxWidth)
            color: Theme.primaryTextColor
            font.pointSize: UiHelper.fixFontSz(15)
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            readOnly: true
            text: chatMessage.body
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
            height: row.height + topPadding + bottomPadding
            width: row.width + leftPadding + rightPadding

            readonly property double topPadding: d.isPicture ? d.defaultRadius : 12
            readonly property double bottomPadding: d.isPicture ? d.defaultRadius : 12
            readonly property double leftPadding: d.isPicture ? d.defaultRadius : 12
            readonly property double rightPadding: d.isPicture ? d.defaultRadius : 12

            RowLayout {
                id: row
                spacing: 10
                x: leftPadding
                y: topPadding

                Rectangle {
                    width: image.width
                    height: image.height
                    color: d.isPicture ? "white" : "transparent"

                    Image {
                        id: image
                        width: sourceSize.width
                        height: sourceSize.height
                        visible: d.isPicture ? true : !progressBar.visible
                        fillMode: Image.PreserveAspectFit
                        source: {
                            if (d.isPicture) {
                                return attachmentThumbnailUrl;
                            }
                            if (attachmentDownloaded) {
                                return "../resources/icons/File Selected Big.png"
                            }
                            return "../resources/icons/File Download Big.png"
                        }
                    }

                    CircleProgressBar {
                        id: progressBar
                        anchors.centerIn: parent
                        size: 40
                        visible: attachmentStatus == Enums.AttachmentStatus.Loading
                        maxValue: attachmentBytesTotal
                        value: attachmentBytesLoaded
                    }
                }

                ColumnLayout {
                    id: column
                    spacing: 4
                    visible: !d.isPicture
                    readonly property double maxWidth: d.maxWidth - row.spacing - image.width - leftPadding - rightPadding

                    Label {
                        text: chatMessage.body
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(16)
                        Layout.maximumWidth: Math.min(implicitWidth, column.maxWidth)
                        elide: "ElideMiddle"
                    }

                    Label {
                        text: attachmentDisplaySize
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(10)
                        Layout.maximumWidth: Math.min(implicitWidth, column.maxWidth)
                        elide: "ElideMiddle"
                    }
                }
            }

            property var contextMenu: ContextMenu {
                compact: true

                Action {
                    text: qsTr("Save As...")
                    onTriggered: chatMessage.saveAttachmentAs()
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
            pointSize: UiHelper.fixFontSz(15)
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
                width: chatMessage.width
                height: loader.item.height
                color: "transparent"

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    property var contextMenu: loader.item.contextMenu

                    onLongPressed: {
                        if (Platform.isMobile) {
                            contextMenu.x = point.position.x
                            contextMenu.y = point.position.y - 40
                            contextMenu.open()
                        }
                    }
                    onTapped: {
                        if (Platform.isMobile) {
                            return
                        }
                        contextMenu.x = eventPoint.position.x
                        contextMenu.y = eventPoint.position.y
                        contextMenu.open()
                        eventPoint.accepted = false
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onClicked: chatMessage.saveAttachmentAs()
                }

                Rectangle {
                    width: chatMessage.width
                    height: loader.item.height
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
                        case "4": return "failed"
                        default: return ""
                    }
                }
                color: chatMessage.failed ? "red" : Theme.labelColor
                font.pixelSize: UiHelper.fixFontSz(11)
            }
        }
    }
}
