import QtQuick 2.12
import QtGraphicalEffects 1.0

import "../theme"
import "./CommonHelpers"

Item {
    property string nickname
    property string avatarUrl
    property alias diameter: textInCircle.diameter
    property alias content: textInCircle.content
    property alias pointSize: textInCircle.pointSize

    width: Theme.avatarHeight
    height: width

    TextInCircle {
        id: textInCircle
        color: Theme.avatarBgColor // intToHexColor(hashCode(nickname))
        anchors.centerIn: parent
        diameter: parent.height
        content: nickname.replace("_", "").substring(0, 2).toUpperCase()
        pointSize: UiHelper.fixFontSz(0.4 * diameter)
        visible: !imageItem.visible

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
        visible: originalImage.status == Image.Ready

        Image {
            id: originalImage
            anchors.fill: parent
            source: avatarUrl
            mipmap: true
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            onStatusChanged: {
                if (status == Image.Error) {
                    console.log("There was an error -> image URL  : ", avatarUrl)
                }
            }
        }

        Rectangle {
            id: rectangleMask
            anchors.fill: parent
            radius: parent.height
            visible: false
        }

        OpacityMask {
            anchors.fill: originalImage
            source: originalImage
            maskSource: rectangleMask
        }
    }
}
