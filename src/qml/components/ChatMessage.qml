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
    property string attachmentSize
    property string attachmentDisplaySize
    property var attachmentType
    property string attachmentLocalUrl
    property string attachmentLocalPreview
    property int attachmentUploaded
    property bool attachmentLoadingFailed

    QtObject {
        id: d
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: isUser ? "#59717D" : Theme.mainBackgroundColor
        readonly property double maxWidth: chatMessage.parent.width - 40
        readonly property bool isPicture: attachmentType == Enums.AttachmentType.Picture
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

            readonly property double topPadding: d.isPicture ? d.defaultRadius : 6
            readonly property double bottomPadding: d.isPicture ? d.defaultRadius : 6
            readonly property double leftPadding: d.isPicture ? d.defaultRadius : 4
            readonly property double rightPadding: d.isPicture ? d.defaultRadius : 10

            RowLayout {
                id: row
                spacing: 4
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
                        fillMode: Image.PreserveAspectFit
                        source: d.isPicture ? attachmentLocalPreview : "../resources/icons/Logo.png"
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

            ProgressBar {
                id: progressBar
                anchors.verticalCenter: d.isPicture ? undefined : parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: d.isPicture ? parent.bottom : undefined
                anchors.bottomMargin: d.isPicture ? 15 : 0

                padding: 2
                width: 0.95 * row.width
                visible: attachmentUploaded < attachmentSize
                from: 0
                to: attachmentSize
                value: attachmentUploaded

                background: Rectangle {
                    implicitWidth: progressBar.width
                    implicitHeight: 6
                    color: "#e6e6e6"
                    radius: 3
                }

                contentItem: Item {
                    implicitWidth: progressBar.width
                    implicitHeight: 4

                    Rectangle {
                        width: progressBar.visualPosition * parent.width
                        height: parent.height
                        radius: 2
                        color: "#17a81a"
                    }
                }
            }

            property var contextMenu: ContextMenu {
                Action {
                    text: qsTr("Save as...")
                    onTriggered: console.log("Feature is not implemented")
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

                Rectangle {
                    width: chatMessage.width
                    height: loader.item.height
                    color: d.background
                    radius: d.isPicture ? 4 : 20
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

                Rectangle {
                    color: "transparent"
                    visible: d.hasAttachment
                }
            }

            // Status label
            Label {
                id: statusLabel
                height: 12
                text: isUser ? "" : chatMessage.status
                color: chatMessage.failed ? "red" : Theme.labelColor
                font.pixelSize: UiHelper.fixFontSz(11)
            }
        }
    }
}
