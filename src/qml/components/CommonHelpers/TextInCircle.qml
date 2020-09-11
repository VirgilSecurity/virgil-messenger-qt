import QtQuick 2.12

Rectangle {
    property alias content: label.text
    property real diameter
    property alias pointSize: label.font.pointSize

    width: diameter
    height: diameter
    radius: diameter / 2

    Text {
        id: label
        text: content
        color: "white"
        font.pointSize: pointSize
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
