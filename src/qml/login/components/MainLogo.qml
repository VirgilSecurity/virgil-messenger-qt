import QtQuick 2.0
import "../../theme"

Column {
    anchors.fill: parent
    spacing: 10

    Image {
        id: mainLogo
        width: 200
        height: 200

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
        text: qsTr("The most secure messenger on the market")
        lineHeight: 1
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        font.pointSize: UiHelper.fixFontSz(13)
        color: Theme.secondaryTextColor
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
