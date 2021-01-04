import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../base"
import "../theme"

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
