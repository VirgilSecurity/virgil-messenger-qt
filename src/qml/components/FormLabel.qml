import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

Text {
    Layout.fillWidth: true
    Layout.maximumWidth: Theme.formMaximumWidth
    Layout.alignment: Qt.AlignHCenter

    color: Theme.labelColor
    font.letterSpacing: 0.4
    font.bold: false
    font.family: Theme.mainFont
    font.pointSize: UiHelper.fixFontSz(12)
    wrapMode: Text.WordWrap
    textFormat: Text.RichText
    linkColor: Theme.buttonPrimaryColor
    onLinkActivated: controllers.documentInteraction.openUrl(link)
}
