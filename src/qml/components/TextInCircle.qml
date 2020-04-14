import QtQuick 2.0

Rectangle {
    property var content
    property real diameter
    property real pointSize

    width: diameter
    height: diameter
    radius: diameter / 2

    Text {
        text: content
        color: "white"
        font.pointSize: pointSize
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
