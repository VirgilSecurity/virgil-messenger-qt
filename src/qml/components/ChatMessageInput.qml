import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

Control {

    signal messageSending(string message)

    width: parent.width
    implicitHeight: scrollView.height

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    RowLayout {

        anchors.fill: parent

        ImageButton {
            Layout.leftMargin: 20
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter
            imageSource: "../resources/icons/Grid.png"
        }

        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.maximumHeight: 100
            // Layout.rightMargin: 15

            TextArea {
                id: messageField
                width: scrollView.width

                placeholderText: qsTr("Message")
                placeholderTextColor: "#59717D"
                wrapMode: TextArea.Wrap
                font.family: Theme.mainFont
                font.pixelSize: 15
                color: Theme.primaryTextColor
                verticalAlignment: TextEdit.AlignVCenter
                leftPadding: 20
                topPadding: 20
                bottomPadding: 20
                // textFormat: "RichText"

                background: Rectangle {
                   anchors.fill: parent
                   anchors.topMargin: 10
                   anchors.bottomMargin: 10
                   radius: 20
                   color: "#37474F"
                }
            }
        }

        ImageButton {
            Layout.rightMargin: 20
            Layout.leftMargin: 10
            Layout.alignment: Qt.AlignVCenter
            objectName: "sendButton"
            imageSource: "../resources/icons/Send.png"
            enabled: messageField.length > 0            
            onClicked: {
                messageSending(messageField.text)
                messageField.text = ""
            }
        }
    }
}
