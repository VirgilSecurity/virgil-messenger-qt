import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../../theme"

Item {
    id: popup

    property alias popupBorderColor: popupBackgroundBorder.color
    property alias popupBackgroundColor: popupBackground.color
    property alias popupColorText: message.color
    property alias popupText: message.text
    property int popupInterval: 4000

    readonly property int defaultMargins: 20

    width: root.width
    height: root.height

    Item {
        id: popupInner
        anchors {
            left: popup.left
            leftMargin: popup.width * 0.1
            right: popup.right
            rightMargin: popup.width * 0.1

            verticalCenter: popup.verticalCenter
            verticalCenterOffset: popup.height * 0.25
        }

        height: 36
        opacity: 0

        Item {
            anchors.centerIn: parent
            width: message.contentWidth + defaultMargins
            height: message.contentHeight + defaultMargins
            layer.enabled: true

            Rectangle {
                id: popupBackgroundBorder
                anchors {
                    fill: parent
                    leftMargin: 1
                    rightMargin:  1
                    bottomMargin:  1
                }
                focus: false
                radius: 6

                Rectangle {
                    id: popupBackground
                    anchors {
                        fill: parent
                        topMargin: 2
                        leftMargin: -1
                        rightMargin:  -1
                        bottomMargin:  -1
                    }
                    radius: parent.radius
                }
            }
        }

        Text {
            id: message
            anchors {
                verticalCenter:  parent.verticalCenter
                left: parent.left
                right: parent.right
            }

            text: popup.popupText
            antialiasing: true
            clip: false
            font.pointSize: Theme.isDesktop ? 11 : 14
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    ParallelAnimation {
        id: showAnimate
        NumberAnimation {target: popupInner; property: "opacity"; from: 0.0; to: 1.0;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1}
        NumberAnimation {target: popupInner; property: "scale"; from: 0.5; to: 1.0;
            easing.type: Easing.OutBack; duration: Theme.animationDuration; easing.overshoot: 2}
    }

    ParallelAnimation {
        id: hideAnimate
        NumberAnimation {target: popupInner; property: "opacity"; from: 1.0; to: 0.0;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1}
        NumberAnimation {target: popupInner; property: "scale"; from: 1.0; to: 0.5;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1}
    }

    function open() {
        showAnimate.start()
        closingTimer.start()
    }

    Timer {
        id: closingTimer
        running: false
        repeat: false
        interval: popup.popupInterval
        onTriggered: {
            hideAnimate.start()
        }
    }
}
