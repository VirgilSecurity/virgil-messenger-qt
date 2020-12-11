import QtQuick 2.12
import QtGraphicalEffects 1.0

import "../theme"
import "./CommonHelpers"

Item {
    property string nickname
    property alias avatarUrl: originalImage.source
    property alias diameter: textInCircle.diameter
    property alias content: textInCircle.content
    property alias pointSize: textInCircle.pointSize

    width: Theme.avatarHeight
    height: width

    TextInCircle {
        id: textInCircle
        color: Theme.avatarBgColor
        anchors.centerIn: parent
        diameter: parent.height
        content: nickname.replace("_", "").substring(0, 2).toUpperCase()
        pointSize: UiHelper.fixFontSz(0.4 * diameter)
        visible: !imageItem.visible
    }

    Item {
        id: imageItem
        anchors.fill: parent
        visible: originalImage.status == Image.Ready

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
            id: originalImage
            anchors.fill: parent
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            onStatusChanged: {
                if (status == Image.Error) {
                    console.log("Avatar url loading error:", source)
                }
            }
        }
    }
}
