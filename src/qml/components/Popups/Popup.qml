import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../../theme"

Item {
    id: popup

    property alias popupBackgroundColor: popupBackground.color
    property alias popupColorText: message.color
    property alias popupText: message.text
    property int popupInterval: 4000

    readonly property int defaultMargins: 20

    width: window.width
    height: window.height
    z: Theme.overlayZ

    Item {
        id: popupInner
        anchors {
            left: popup.left
            right: popup.right
        }

        height: 36
        opacity: 0

        Rectangle {
            id: popupBackground
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }

            width: parent.width
            height: message.contentHeight + defaultMargins

            focus: false

            Text {
                id: message
                anchors.centerIn: parent
                width: popupInner.width
                padding: defaultMargins
                text: popup.popupText
                antialiasing: true
                clip: false
                font.pointSize: Theme.isDesktop ? 11 : 14
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    ParallelAnimation {
        id: showAnimate
        NumberAnimation {target: popupInner; property: "opacity"; from: 0.0; to: 1.0;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1}
        NumberAnimation {target: popupInner; property: "y"; from: -popupBackground.height; to: 0;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 2}
    }

    ParallelAnimation {
        id: hideAnimate
        NumberAnimation {target: popupInner; property: "opacity"; from: 1.0; to: 0.0;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1}
        NumberAnimation {target: popupInner; property: "y"; from: 0; to: -popupBackground.height;
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
