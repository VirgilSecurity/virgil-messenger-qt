import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    property string variant
    property alias label: label.text
    color: variant === "error" ? "red" : "gray"
    height: visible ? 25 : 0

    Label {
        id: label
        anchors.centerIn: parent
        color: "white"
        font.pointSize: UiHelper.fixFontSz(12)
    }
}
