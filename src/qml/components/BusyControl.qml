import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Item {
    id: busyControl

//    To use - place in center of parent, toggle busy property

    width: size
    height: size

    property color primaryColor: Theme.brandColor
    property color secondaryColor: "transparent"

    property int size: Theme.avatarHeight
    property int lineWidth: 3
    property real value: 100
    property real maxValue: 100
    property bool busy: false
    readonly property real ratio: value / maxValue

    onBusyChanged: {
        if (busy) {
            busyControl.opacity = 1
        } else {
            busyControl.opacity = 0
        }
    }

    onRatioChanged: {
        canvas.degree = ratio * 360;
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
    }

    Behavior on value {
        NumberAnimation { duration: 1000; easing.type: Easing.InOutCubic }
    }

    Timer {
        running: busyControl.busy
        repeat: true
        interval: 1050
        triggeredOnStart: true
        onTriggered: {
            if (busyControl.value === 0) {
                busyControl.value = busyControl.maxValue
            } else {
                busyControl.value = 0
                rotationAnimation.restart()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: height
        color: "black"
        opacity: 0.333
    }

    Canvas {
        id: canvas

        property real degree: 0

        anchors.fill: parent
        antialiasing: true
        opacity: {
            if (value > 15) {
                return 1
            } else {
                return value / 15
            }
        }

        onDegreeChanged: {
            requestPaint();
        }

        NumberAnimation {
            id: rotationAnimation
            target: canvas
            property: "rotation"
            from: 0
            to: 360
            duration: 1000
            easing.type: Easing.InOutCubic
        }

        onPaint: {
            var ctx = getContext("2d");

            var x = busyControl.width/2;
            var y = busyControl.height/2;

            var radius = busyControl.size/2 - busyControl.lineWidth
            var startAngle = (Math.PI/180) * 270;
            var fullAngle = (Math.PI/180) * (270 + 360);
            var progressAngle = (Math.PI/180) * (270 + degree);

            ctx.reset()

            ctx.lineCap = 'round';
            ctx.lineWidth = busyControl.lineWidth;
            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, fullAngle);
            ctx.strokeStyle = busyControl.secondaryColor;
            ctx.stroke();

            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, progressAngle);
            ctx.strokeStyle = busyControl.primaryColor;
            ctx.stroke();
        }
    }
}


