import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Control {
    id: chatMessage

    topPadding: 10
    leftPadding: 16
    rightPadding: 16
    bottomPadding: 12
    height: textEdit.height + 16
    width: getControlWidth()

    property alias text: textEdit.text
    property alias author: avatar.nickname
    property var variant
    property var timeStamp

    Row {

        spacing: 12

        Avatar {
            id: avatar
            width: 30
            diameter: 30
            pointSize: UiHelper.fixFontSz(15)
        }

        Column {
            spacing: 4
            RowLayout {
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

            Control {
                width: chatMessage.width
                height: textEdit.height

                background: Rectangle {
                    width: chatMessage.width
                    height: textEdit.height
                    color: chatMessage.variant === "dark"
                           ? Theme.mainBackgroundColor
                           : "#59717D"
                    radius: 20

                    Rectangle {
                        height: 20
                        width: 20
                        radius: 4
                        color: chatMessage.variant === "dark"
                               ? Theme.mainBackgroundColor
                               : "#59717D"
                    }
                }

                TextEdit {
                    id: textEdit
                    topPadding: chatMessage.topPadding
                    leftPadding: chatMessage.leftPadding
                    rightPadding: chatMessage.rightPadding
                    bottomPadding: chatMessage.bottomPadding
                    textFormat: Text.RichText
                    width: chatMessage.width
                    color: Theme.primaryTextColor
                    font.pixelSize: UiHelper.fixFontSz(20)
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    selectByMouse: true
                    readOnly: true
                    text: isValidURL(message) ? ("<a href='"+message+"'>"+message+"</a>") : message
                    onLinkActivated:{
                        if (isValidURL(message)){
                           Qt.openUrlExternally(message)
                        }
                    }

                    function isValidURL(message) {
                       var regexp = /(ftp|http|https):\/\/(\w+:{0,1}\w*@)?(\S+)(:[0-9]+)?(\/|\/([\w#!:.?+=&%@!\-\/]))?/
                       return regexp.test(message);
                    }
                }
            }
        }
    }

    function getControlWidth() {
        if ((textEdit.implicitWidth + 40) > parent.width) {
            return parent.width - 40
        }

        return textEdit.implicitWidth
    }
}
