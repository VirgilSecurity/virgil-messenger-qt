import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Rectangle {
    id: root
    color: "black"
    opacity: 0
    visible: false

    Image {
        id: imagePreview
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

        onClicked: root.closePreview()
    }

    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent

        onClicked: root.closePreview()
        onWheel: wheel.accepted = true
    }

    ParallelAnimation {
        id: showAnimation
        NumberAnimation { target: attachmentPreview; property: "opacity"; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.OutCubic}
        NumberAnimation { target: imagePreview; property: "scale"; from: 0.5; to: 1; duration: Theme.animationDuration; easing.type: Easing.OutCubic}

        onStarted: root.visible = true
    }

    ParallelAnimation {
        id: hideAnimation
        NumberAnimation { target: attachmentPreview; property: "opacity"; to: 0; duration: Theme.shortAnimationDuration; easing.type: Easing.OutCubic}

        onFinished: root.visible = false
    }

    function openPreview(url) {
        imagePreview.source = url
        showAnimation.restart()
    }

    function closePreview() { hideAnimation.restart() }

    function navigateBack(transition) {
        if (visible) {
            closePreview()
            return true
        }
        return false
    }

    Component.onCompleted: controllers.attachments.openPreviewRequested.connect(openPreview)
}
