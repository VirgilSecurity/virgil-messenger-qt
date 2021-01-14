import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"

Item {
    id: attachmentPreview
    onVisibleChanged: startAnimation.restart()

    Rectangle {
        id: imageBackground
        anchors.fill: parent
        color: "black"
        opacity: 0
    }

    Image {
        id: imagePreview
        source: app.stateManager.attachmentPreviewState.url
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        autoTransform: true
    }

    Label {
        anchors {
            top: parent.top
            right: parent.right
            topMargin: 15
            rightMargin: 15
        }
        text: qsTr("Close")
        color: "white"
        font.pointSize: 14
    }

    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent
        onClicked: app.stateManager.goBack()
        onWheel: wheel.accepted = true
    }

    ParallelAnimation {
        id: startAnimation
        NumberAnimation { target: imageBackground; property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.OutCubic}
        NumberAnimation { target: imagePreview; property: "scale"; from: 0.5; to: 1; duration: Theme.animationDuration; easing.type: Easing.OutCubic}
    }
}
