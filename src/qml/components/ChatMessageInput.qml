import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.platform 1.0 as Platform

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
                selectByMouse: true
                selectedTextColor: "black"
                selectionColor: "white"
                // textFormat: "RichText"

                 background: Rectangle {
                   anchors.fill: parent
                   anchors.topMargin: 10
                   anchors.bottomMargin: 10
                   radius: 20
                   color: "#37474F"
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    hoverEnabled: true
                    onClicked: {
                        const selectStart = messageField.selectionStart;
                        const selectEnd = messageField.selectionEnd;
                        const curPos = messageField.cursorPosition;
                        contextMenu.open();
                        messageField.cursorPosition = curPos;
                        messageField.select(selectStart, selectEnd);
                    }
                    onPressAndHold: {
                        if (mouse.source === Qt.MouseEventNotSynthesized) {
                            const selectStart = messageField.selectionStart;
                            const selectEnd = messageField.selectionEnd;
                            const curPos = messageField.cursorPosition;
                            contextMenu.open();
                            messageField.cursorPosition = curPos;
                            messageField.select(selectStart, selectEnd);
                        }
                    }

                    Platform.Menu {
                        id: contextMenu
                        Platform.MenuItem {
                            text: "Cut"
                            onTriggered: {
                                messageField.cut()
                            }
                        }
                        Platform.MenuItem {
                            text: "Copy"
                            onTriggered: {
                                messageField.copy()
                            }
                        }
                        Platform.MenuItem {
                            text: "Paste"
                            onTriggered: {
                                messageField.paste()
                            }
                        }
                    }
                }
            }
        }

        ImageButton {
            Layout.rightMargin: 20
            Layout.leftMargin: 10
            Layout.alignment: Qt.AlignVCenter            
            focusPolicy: Qt.NoFocus
            objectName: "btnSend"            
            imageSource: "../resources/icons/Send.png"
            onClicked: {
                const text = messageField.text + messageField.preeditText;

                if (text.trim().length > 0) {
                    messageSending(text.trim())
                }

                messageField.clear()
            }
        }
    }
}
