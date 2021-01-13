import QtQuick 2.12

import "../theme"

// Original source: https://github.com/rafzby/circular-progressbar

Item {
    id: root

    property int size: 100
    property int lineWidth: 3
    property real value: 0
    property real maxValue: 0
    property bool animated: false

    property color primaryColor: Theme.brandColor
    property color secondaryColor: "transparent"

    width: size
    height: size

    readonly property real ratio: value / maxValue

    onRatioChanged: {
        canvas.degree = ratio * 360;
    }

    Behavior on value {
        NumberAnimation { duration: Theme.animationDuration }
    }

    Rectangle {
        anchors.fill: parent
        radius: height
        color: "black"
        opacity: 0.333
    }

    Repeater {
        model: 2
        Rectangle {
            anchors.centerIn: parent
            width: 0.5 * size
            height: lineWidth
            radius: height
            color: primaryColor
            rotation: index ? -45 : 45
        }
    }

    Canvas {
        id: canvas

        property real degree: 0

        anchors.fill: parent
        antialiasing: true

        onDegreeChanged: {
            requestPaint();
        }

        RotationAnimation on rotation {
            loops: Animation.Infinite
            from: 0
            to: 360
            duration: 2000
            running: root.animated
        }

        onPaint: {
            var ctx = getContext("2d");

            var x = root.width/2;
            var y = root.height/2;

            var radius = root.size/2 - root.lineWidth
            var startAngle = (Math.PI/180) * 270;
            var fullAngle = (Math.PI/180) * (270 + 360);
            var progressAngle = (Math.PI/180) * (270 + degree);

            ctx.reset()

            ctx.lineCap = 'round';
            ctx.lineWidth = root.lineWidth;
            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, fullAngle);
            ctx.strokeStyle = root.secondaryColor;
            ctx.stroke();

            ctx.beginPath();
            ctx.arc(x, y, radius, startAngle, progressAngle);
            ctx.strokeStyle = root.primaryColor;
            ctx.stroke();
        }
    }
}
