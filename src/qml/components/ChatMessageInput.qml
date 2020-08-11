import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.platform 1.0 as Native
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"

Control {
    id: root

    signal messageSending(string message, var attachmentUrl, var attachmentType)

    width: parent.width
    implicitHeight: scrollView.height

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    RowLayout {
        anchors.fill: parent

        ImageButton {
            Layout.leftMargin: 12
            Layout.rightMargin: 2
            Layout.alignment: Qt.AlignVCenter
            image: "Grid"

            onClicked: attachmentsMenu.open()

            ContextMenu {
                id: attachmentsMenu
                dropdown: true

// FIXME(fpohtmeh): restore
//                Action {
//                    text: qsTr("Send picture")
//                    onTriggered: selectAttachment(Enums.AttachmentType.Picture)
//                }

                Action {
                    text: qsTr("Send file")
                    onTriggered: selectAttachment(Enums.AttachmentType.File)
                }
            }
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

                Keys.onPressed: {
                    if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
                        if (!Platform.isDesktop) {
                            return
                        }
                        if (event.modifiers == Qt.ShiftModifier) {
                            // TextArea adds newline here
                        }
                        else if (event.modifiers == Qt.ControlModifier) {
                            // Adds new line. Same behaviour as for Shift+Enter
                            messageField.remove(messageField.selectionStart, selectionEnd)
                            messageField.insert(messageField.selectionStart, "\n")
                            event.accepted = true
                        }
                        else {
                            event.accepted = true
                            root.sendMessage()
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    hoverEnabled: true
                    onClicked: messageField.openContextMenu()
                    onPressAndHold: {
                        if (mouse.source === Qt.MouseEventNotSynthesized)
                            messageField.openContextMenu()
                    }

                    Native.Menu {
                        id: contextMenu
                        Native.MenuItem {
                            text: "Cut"
                            onTriggered: {
                                messageField.cut()
                            }
                        }
                        Native.MenuItem {
                            text: "Copy"
                            onTriggered: {
                                messageField.copy()
                            }
                        }
                        Native.MenuItem {
                            text: "Paste"
                            onTriggered: {
                                messageField.paste()
                            }
                        }
                    }
                }

                function openContextMenu() {
                    const selStart = selectionStart;
                    const selEnd = selectionEnd;
                    const curPos = cursorPosition;
                    contextMenu.open();
                    messageField.cursorPosition = curPos;
                    messageField.select(selStart, selEnd);
                }
            }
        }

        ImageButton {
            id: sendButton
            Layout.rightMargin: 12
            Layout.leftMargin: 2
            Layout.alignment: Qt.AlignVCenter
            focusPolicy: Qt.NoFocus
            objectName: "btnSend"
            disabled: !(messageField.text + messageField.preeditText).length
            image: "Send"
            onClicked: root.sendMessage()
        }
    }

    SelectAttachmentsDialog {
        id: fileDialog

        onAccepted: sendMessage(fileDialog.fileUrl, fileDialog.attachmentType)
    }

    function sendMessage(attachmentUrl, attachmentType) {
        const text = (messageField.text + messageField.preeditText).trim();
        messageField.clear()
        if (text || attachmentUrl)
            messageSending(text, attachmentUrl, attachmentType)
    }

    function selectAttachment(attachmentType) {
        fileDialog.attachmentType = attachmentType
        fileDialog.open()
    }
}
