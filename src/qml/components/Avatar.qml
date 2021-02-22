import QtQuick 2.12
import QtGraphicalEffects 1.0

import "../theme"
import "./CommonHelpers"

Item {
    id: avatar
    property string nickname
    property alias avatarUrl: originalImage.source
    property alias diameter: avatar.width
    property alias content: textInCircle.content
    property alias pointSize: textInCircle.pointSize

    property bool isSelected: false

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

        Image {
            id: originalImage
            anchors.fill: parent
            source: avatarUrl
            smooth: true
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            visible: false
            onStatusChanged: {
                if (status == Image.Error) {
                    console.log("Avatar url loading error:", avatarUrl)
                }
            }
        }

        Rectangle {
            id: rectangleMask
            width: parent.width
            height: parent.height
            radius: height
            visible: false
        }

        OpacityMask {
            anchors.fill: originalImage
            source: originalImage
            maskSource: rectangleMask
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width + 8
        height: width
        visible: isSelected
        radius: width
        color: 'transparent'
        border.width: 2
        border.color: Theme.contactSelectionColor

        Rectangle {
            anchors {
                bottom: parent.bottom
                right: parent.right
            }

            width: parent.width * 0.38
            height: width
            radius: width
            color: Theme.contactSelectionColor

            border.width: 2
            border.color: Theme.mainBackgroundColor

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.7
                fillMode: Image.PreserveAspectFit
                source: "../resources/icons/Check-XSmall.png"
            }
        }
    }

}
