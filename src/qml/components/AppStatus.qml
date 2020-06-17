import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    property string variant
    color: variant === "error" ? "red" : "gray"

    Label {
        id: label
        anchors.centerIn: parent
        color: "white"
        font.pointSize: UiHelper.fixFontSz(12)
    }

    function show(text, interval) {
        if (interval) {
            closeTimer.interval = interval
            closeTimer.start()
        }

        label.text = text;
        height = 25
    }

    function hide() {
        height = 0
        label.text = ""
    }

    Timer {
        id: closeTimer
        onTriggered: hide()
    }
}
