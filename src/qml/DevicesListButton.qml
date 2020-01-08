// SquareButton.qml
import QtQuick 2.0

Rectangle {
    property var side: 100
    signal clicked()

    id: devicesListButton
    width: side * 0.95
    height: side * 0.95
    color: "darkblue"

    Text {
        anchors.centerIn: parent
        renderType: Text.NativeRendering
        color: "white"
        font.pixelSize: side * 0.25
        text: "Devices"
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
        devicesListButton.clicked();
        }
    }
}