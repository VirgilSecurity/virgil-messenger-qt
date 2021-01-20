import QtQuick 2.12
import QtQuick.Controls 2.12

import "../theme"

Rectangle {
    id: attachmentPreview
    onVisibleChanged: showAnimation.restart()
    color: "black"
    opacity: 0

    Image {
        id: imagePreview
        source: app.stateManager.attachmentPreviewState.url
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        autoTransform: true
    }

    ImageButton {
        anchors {
            top: parent.top
            right: parent.right
            topMargin: 15
            rightMargin: 15
        }
        image: "Close"

        onClicked: {
            hideAnimation.restart()
            goBackTimer.restart()
        }
    }

    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent
        onClicked: {
            hideAnimation.restart()
            goBackTimer.restart()
        }

        onWheel: wheel.accepted = true
    }

    Timer {
        id: goBackTimer
        running: false
        repeat: false
        interval: Theme.shortAnimationDuration
        onTriggered: {
            app.stateManager.goBack()
        }
    }

    ParallelAnimation {
        id: showAnimation
        NumberAnimation { target: attachmentPreview; property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.OutCubic}
        NumberAnimation { target: imagePreview; property: "scale"; from: 0.5; to: 1; duration: Theme.animationDuration; easing.type: Easing.OutCubic}
    }

    ParallelAnimation {
        id: hideAnimation
        NumberAnimation { target: attachmentPreview; property: "opacity"; to: 0; duration: Theme.shortAnimationDuration; easing.type: Easing.OutCubic}
    }
}
