import QtQuick 2.15

import "../theme"

Item {
    width: height

    Image {
        id: imagePreview
        source: "../resources/icons/%1.png".arg("Overlay-Offline")
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        autoTransform: true
        visible: setIconVisible()
    }

    Rectangle {
        id: networkRectangle
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: 2
            bottomMargin: 2
        }

        width: 6
        height: width
        radius: width * 0.5
        color: setColor()
    }

    function setColor() {
        if (messenger.connectionStateString === "connected") {
            return Theme.labelColor
        }
        if (messenger.connectionStateString === "connecting") {
            return "orange"
        }
        return "transparent"
    }

    function setIconVisible() {
        if (messenger.connectionStateString === "connected") {
            return false
        }
        if (messenger.connectionStateString === "connecting") {
            return false
        }
        return true
    }
}
