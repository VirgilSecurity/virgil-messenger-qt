import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: attachmentPreview

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.5
    }

    Image {
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
        font.bold: true
    }

    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent
        onClicked: app.stateManager.goBack()
        onWheel: wheel.accepted = true
    }
}
