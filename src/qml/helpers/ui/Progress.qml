import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4

Page {
    id: progressPage

    // Signals
    signal done()

    // Properties
    property alias timerStart: simpleTimer.running
    property string titleStr: ""
    property int reqTimeMs: 0
    readonly property int tickMs: 30

    onVisibleChanged: {
        progressBar.value = 0
        simpleTimer.running = visible
    }

    background: Rectangle {
        color: backGroundColor
    }

    Text {
        id: titleText
        text: titleStr
        font.pointSize: 24
        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        color: mainTextCOlor
    }

    ColumnLayout {
        width: parent.width
        anchors.top: titleText.bottom
        anchors.topMargin: 30
        spacing: 25

        ProgressBar {
            id: progressBar
            from: 0
            to: reqTimeMs / tickMs
            value: 0
            padding: 2

            height: 20
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width - 20

            background: Rectangle {
                anchors.left: progressBar.left
                anchors.verticalCenter: progressBar.verticalCenter
                implicitWidth: 50
                implicitHeight: 20
                color: "white"
                radius: 3
            }

            contentItem: Rectangle {
                anchors.verticalCenter: progressBar.verticalCenter
                width: progressBar.visualPosition * progressBar.width
                radius: 2
                color: backGroundColor
            }
        }

        // Timer to show off the progress bar
        Timer {
            id: simpleTimer
            interval: tickMs
            repeat: true
            running: false
            onTriggered: {
                if (progressBar.value < progressBar.to) {
                    progressBar.value += 1.0
                } else {
                    progressBar.value = progressBar.to
                    timeout()
                }
            }
        }

        Item {
            height: 20
        }

        CButton {
            height: 50
            Layout.preferredWidth: parent.width - 20
            Layout.alignment: Qt.AlignHCenter
            name: qsTr("Cancel")
            baseColor: "transparent"
            borderColor: mainAppColor
            onClicked: stackView.pop()
        }
    }

    function timeout() {
        simpleTimer.running = false
        progressPage.done()
    }
}
