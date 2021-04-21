import QtQuick 2.15

import "../theme"

Rectangle {
    height: 8
    width: height
    radius: 0.5 * width
    z: Theme.overlayZ
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
