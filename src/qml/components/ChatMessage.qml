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

    property string text
    property alias nickname: avatar.nickname

    property bool messageInARow: false
    property bool firstMessageInARow: true
    property var variant
    property var timeStamp
    property string status: ""

    property string attachmentId
    property string attachmentSize
    property var attachmentType
    property string attachmentLocalUrl
    property string attachmentLocalPreview

    QtObject {
        id: d
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: variant === "dark" ? Theme.mainBackgroundColor : "#59717D"
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
            text: chatMessage.text.split("\n").join("<br />")
            // TODO(fpohtmeh): remove
            // text: isValidURL(message) ? ("<a href='"+message+"'>"+message+"</a>") : message
            visible: !d.hasAttachment

            onLinkActivated: {
                if (isValidURL(message)){
                   Qt.openUrlExternally(message)
                }
            }

            function isValidURL(message) {
               var regexp = /(ftp|http|https):\/\/(\w+:{0,1}\w*@)?(\S+)(:[0-9]+)?(\/|\/([\w#!:.?+=&%@!\-\/]))?/
               return regexp.test(message);
            }

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
                        text: chatMessage.text
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(16)
                        Layout.maximumWidth: Math.min(implicitWidth, column.maxWidth)
                        elide: "ElideMiddle"
                    }

                    Label {
                        text: attachmentSize
                        color: "white"
                        font.pixelSize: UiHelper.fixFontSz(10)
                        Layout.maximumWidth: Math.min(implicitWidth, column.maxWidth)
                        elide: "ElideMiddle"
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
            opacity: firstMessageInARow ? 1 : 0
            diameter: 30
            pointSize: UiHelper.fixFontSz(15)
        }

        Column {
            spacing: 4

            // Nickname + timestamp
            RowLayout {
                visible: firstMessageInARow
                spacing: 6

                Label {
                    text: nickname
                    height: 16
                    color: Theme.labelColor
                    font.pixelSize: UiHelper.fixFontSz(16)
                }

                Label {
                    Layout.alignment: Qt.AlignBottom
                    text: "•  %1".arg(Qt.formatDateTime(timeStamp, "hh:mm"))
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
                    visible: messageInARow
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
                text: getStatusById(status)
                color: status === "4" ? "red" : Theme.labelColor
                font.pixelSize: UiHelper.fixFontSz(11)
            }
        }
    }

    function getStatusById(statusId) {
        switch (statusId) {
            // TODO(fpohtmeh): get from model
            case "0": return "sending"
            case "1": return "sent"
            case "2": return "delivered"
            case "4": return "failed"
            default: return ""
        }
    }
}
