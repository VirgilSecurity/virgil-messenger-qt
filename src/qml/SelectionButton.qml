// SquareButton.qml
import QtQuick 2.0

Rectangle {
    property int side: applicationWindow.footerHeight - 2 * applicationWindow.margin
    property int textFontSize
    property string buttonText
    property bool isSniffer
    signal clicked()

    width: side * 0.95
    height: side * 0.95
    color: "darkblue"

    Text {
        anchors.centerIn: parent
        renderType: Text.NativeRendering
        color: "white"
        font.pixelSize: applicationWindow.dp(15)
        text: buttonText
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            applicationWindow.buttonClicked(isSniffer);
        }
    }
}