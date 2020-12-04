import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../../theme"

Item {
    id: popup

    property alias popupBorderColor: popupBackground.border.color
    property alias popupBackgroundColor: popupBackground.color
    property alias popupColorText: message.color
    property alias popupText: message.text
    property int popupInterval: 3000

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

        Rectangle {
            id: popupBackground
            anchors.centerIn: parent
            width: message.contentWidth + 18
            height: message.contentHeight + 18
            focus: false
            color: "black"
            border.width: 2
            opacity: 0.8
            radius: 12
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
            color: 'white'
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
        NumberAnimation {target: popupInner; property: "scale"; from: 0.0; to: 1.0;
            easing.type: Easing.OutBack; duration: Theme.animationDuration; easing.overshoot: 2}
    }

    ParallelAnimation {
        id: hideAnimate
        NumberAnimation {target: popupInner; property: "opacity"; from: 1.0; to: 0.0;
            easing.type: Easing.InOutQuad; duration: Theme.animationDuration; easing.overshoot: 1}
        NumberAnimation {target: popupInner; property: "scale"; from: 1.0; to: 0.0;
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

    // old code
    //    property alias popupView: popup
    //    property alias popupColor: popupBackground.color
    //    property alias popupColorText: message.color
    //    property bool popupModal: true
    //    property bool popupOnTop: false

    //    Popup {
    //        id: popup

    //        property alias popMessage: message.text

    //        background: Rectangle {
    //            id: popupBackground
    //            implicitWidth: root.width
    //            implicitHeight: 25
    //        }

    //        enter: Transition {
    //            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
    //        }

    //        exit: Transition {
    //            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
    //        }

    //        onAboutToHide: {
    //            popupClose.stop()
    //        }

    //        y: popupOnTop ? 0 : (root.height - 25)
    //        modal: popupModal
    //        focus: popupModal

    //        closePolicy: Popup.CloseOnPressOutside

    //        Text {
    //            id: message
    //            anchors.centerIn: parent
    //            font.pointSize: UiHelper.fixFontSz(12)
    //        }
    //        onOpened: popupClose.start()
    //    }

    //    // Popup will be closed automatically in 2 seconds after its opened
    //    Timer {
    //        id: popupClose
    //        interval: 2000
    //        onTriggered: popup.close()
    //    }
}

