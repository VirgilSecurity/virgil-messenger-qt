import QtQuick 2.12
import QtQuick.Layouts 1.12

import "../theme"

Column {

    Layout.fillWidth: true
    Layout.maximumWidth: Theme.formMaximumWidth
    Layout.alignment: Qt.AlignHCenter

    spacing: 10

    Image {
        id: mainLogo
        width: 96
        height: 96

        fillMode: Image.PreserveAspectFit
        source: Theme.mainLogo
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: mainLogoText
        text: Theme.mainLogoText
        font.weight: Font.Bold
        font.capitalization: Font.Capitalize
        font.family: Theme.mainFontBold
        font.pointSize: UiHelper.fixFontSz(48)
        color: Theme.brandColor
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: mainLogoDescription
        width: 180
        text: qsTr("The Most secure messenger on the market")
        lineHeight: 1
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        font.pointSize: UiHelper.fixFontSz(13)
        font.letterSpacing: 0.3
        color: Theme.menuSeparatorColor
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
