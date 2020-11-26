import QtQuick 2.12
import QtGraphicalEffects 1.0

import "../theme"
import "./CommonHelpers"

Item {
    property string nickname
    property string avatarUrl

    width: Theme.avatarHeight
    height: width

    TextInCircle {
        color: Theme.avatarBgColor // intToHexColor(hashCode(nickname))
        anchors.centerIn: parent
        diameter: parent.height
        content: nickname.replace("_", "").substring(0, 2).toUpperCase()
        pointSize: UiHelper.fixFontSz(0.4 * diameter)

        // Hash any string into an integer value
        // Then we'll use the int and convert to hex.
        function hashCode(str) {
            var hash = 0;
            for (var i = 0; i < str.length; i++) {
                hash = str.charCodeAt(i) + ((hash << 5) - hash);
            }
            return hash;
        }

        // Convert an int to hexadecimal with a max length
        // of six characters.
        function intToHexColor(i) {
            var hex = ((i>>24)&0xFF).toString(16) +
                    ((i>>16)&0xFF).toString(16) +
                    ((i>>8)&0xFF).toString(16) +
                    (i&0xFF).toString(16);

            // Sometimes the string returned will be too short so we
            // add zeros to pad it out, which later get removed if
            // the length is greater than six.
            hex += '000000';
            return "#" + hex.substring(0, 6);
        }
    }

    Item {
        id: imageItem
        anchors.fill: parent
        visible: avatarUrl !== undefined
        layer.enabled: visible
        layer.effect: OpacityMask {
            maskSource: Item {
                width: imageItem.width
                height: imageItem.height
                Rectangle {
                    anchors.fill: parent
                    radius: parent.height
                }
            }
        }

        Image {
            anchors.fill: parent
            source: avatarUrl
            mipmap: true
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
        }
    }
}
