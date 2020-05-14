import QtQuick 2.12

Column {
    anchors.centerIn: parent
    spacing: 10

    property alias image: imageId
    property alias label: labelId

    Image {
        id: imageId
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Text {
        id: labelId
        textFormat: Text.RichText
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: UiHelper.fixFontSz(15)
    }
}
