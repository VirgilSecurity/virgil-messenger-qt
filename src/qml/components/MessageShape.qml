import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

import "../base"
import "../theme"

Rectangle {
    id: paintingRec
    visible: !d.isPicture
    color: d.background
    radius: stdRadiusHeight

    transform: Scale {
        origin.x: paintingRec.width / 2
        origin.y: paintingRec.height / 2
        xScale: isMessageAlignedLeft ? 1 : -1
    }

    Shape {
        id: messageShape
        anchors.fill: parent
        layer.enabled: true
        layer.samples: 4

        ShapePath {
            strokeColor: "transparent"
            strokeWidth: 0
            fillColor: d.background
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

             startX: 0
             startY: messageShape.height * 0.5
             PathLine {x: 0; y: 4}

             PathQuad {x: 4; y: 0; controlX: 0; controlY: 0}

             PathLine {x: messageShape.width * 0.5; y: 0}

             PathLine {x: messageShape.width * 0.5; y: messageShape.height}

             PathLine {x: leftBottomRadiusHeight; y: messageShape.height}

             PathQuad {x: 0; y: messageShape.height - leftBottomRadiusHeight; controlX: 0; controlY: messageShape.height}
        }
    }
}
