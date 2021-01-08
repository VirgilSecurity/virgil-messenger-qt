import QtQuick 2.15

Rectangle {
    height: 8
    width: height
    radius: 0.5 * width
    color: {
        if (messenger.connectionStateString === "connected") {
            return "green";
        }
        if (messenger.connectionStateString === "connecting") {
            return "orange";
        }
        return "red";
    }
}
