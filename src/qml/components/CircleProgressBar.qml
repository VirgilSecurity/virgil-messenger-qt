import QtQuick 2.12

// Original source: https://github.com/rafzby/circular-progressbar

Item {
    id: root

    property int size: 100
    property int lineWidth: 4
    property real value: 0
    property real maxValue: 0
    property bool animated: false

    property color primaryColor: "#29b6f6"
    property color secondaryColor: "#ffffff"

    width: size
    height: size

    readonly property real ratio: value / maxValue

    onRatioChanged: {
        canvas.degree = ratio * 360;
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
