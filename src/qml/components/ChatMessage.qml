import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"


Control {
    id: chatMessage
    height: row.implicitHeight
    width: getControlWidth()

    property string text
    property alias author: avatar.nickname

    property bool messageInARow: false
    property bool firstMessageInARow: true
    property var variant
    property var timeStamp
    property string status: ""

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

            RowLayout {
                visible: firstMessageInARow
                spacing: 6

                Label {
                    text: author
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

            Rectangle {
                width: chatMessage.width
                height: textEdit.height

                color: "transparent"

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onLongPressed: {
                        if (Qt.platform.os == "android" || Qt.platform.os == "ios") {
                            contextMenu.x = point.position.x
                            contextMenu.y = point.position.y - 40

                            contextMenu.open()
                        }
                    }
                    onTapped: {
                        if (Qt.platform.os == "android" || Qt.platform.os == "ios") {
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
                    height: textEdit.height
                    color: chatMessage.variant === "dark" ? Theme.mainBackgroundColor : "#59717D"
                    radius: 20
                }

                Rectangle {
                    anchors.top: parent.top
                    height: 22
                    width: 22
                    radius: 4
                    color: chatMessage.variant === "dark" ? Theme.mainBackgroundColor : "#59717D"
                }

                Rectangle {
                    visible: messageInARow
                    anchors.bottom: parent.bottom
                    height: 22
                    width: 22
                    radius: 4
                    color: chatMessage.variant === "dark" ? Theme.mainBackgroundColor : "#59717D"
                }

                TextEdit {
                    id: textEdit
                    topPadding: 12
                    leftPadding: 15
                    rightPadding: 15
                    bottomPadding: 12
                    enabled: false
                    textFormat: Text.RichText
                    width: chatMessage.width
                    color: Theme.primaryTextColor
                    font.pointSize: UiHelper.fixFontSz(15)
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: chatMessage.text.split("\n").join("<br />")
                    // text: isValidURL(message) ? ("<a href='"+message+"'>"+message+"</a>") : message
                    onLinkActivated:{
                        if (isValidURL(message)){
                           Qt.openUrlExternally(message)
                        }
                    }

                    function isValidURL(message) {
                       var regexp = /(ftp|http|https):\/\/(\w+:{0,1}\w*@)?(\S+)(:[0-9]+)?(\/|\/([\w#!:.?+=&%@!\-\/]))?/
                       return regexp.test(message);
                    }

                    Menu {
                        id: contextMenu
                        MenuItem {
                            text: qsTr("&Copy")
                            onTriggered: {
                                clipboard.setText(textEdit.getText(0, textEdit.length))
                            }
                        }
                    }
                }
            }

            Label {
                id: statusLabel
                height: 16
                text: getStatusById(status)
                color: Theme.labelColor
                font.pixelSize: UiHelper.fixFontSz(12)
            }
        }
    }

    function getStatusById(statusId) {

        console.log(statusId)

        switch (statusId) {
            case "0": return "sending"
            case "1": return "sent"
            case "2": return "delivered"
            case "4": return "failed"
            default: return ""
        }
    }

    function getControlWidth() {
        if ((textEdit.implicitWidth + 40) > parent.width) {
            return parent.width - 40
        }

        return textEdit.implicitWidth
    }
}
