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
    implicitHeight: inputItem.height

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

                Action {
                    text: qsTr("Send picture")
                    onTriggered: selectAttachment(Enums.AttachmentType.Picture)
                }

                Action {
                    text: qsTr("Send file")
                    onTriggered: selectAttachment(Enums.AttachmentType.File)
                }
            }
        }

        Item {
            id: inputItem
            Layout.fillWidth: true
            Layout.maximumHeight: 100
            Layout.preferredHeight: messageField.implicitHeight + 40 // 2 * (margin + padding)

            Rectangle {
                id: inputRect
                anchors {
                    fill: parent
                    topMargin: 10
                    bottomMargin: 10
                }
                radius: 20
                color: "#37474F"

//                border {
//                    color: "cyan"
//                    width: 1
//                }

                ScrollView {
                    id: scrollView
                    anchors {
                        fill: parent
                        topMargin: 10
                        bottomMargin: 10
                        leftMargin: 15
                        rightMargin: 15 - scrollBarWidth
                    }

                    readonly property real scrollBarWidth: ScrollBar.vertical.width

                    TextArea {
                        id: messageField
                        width: scrollView.width
                        padding: 0
                        rightPadding: scrollView.scrollBarWidth
                        placeholderText: qsTr("Message")
                        placeholderTextColor: "#59717D"
                        wrapMode: TextArea.Wrap
                        font.family: Theme.mainFont
                        font.pixelSize: 15
                        color: Theme.primaryTextColor
                        verticalAlignment: TextEdit.AlignVCenter
                        selectByMouse: true
                        selectedTextColor: "black"
                        selectionColor: "white"

//                        Rectangle {
//                            anchors.fill: parent
//                            color: "transparent"
//                            enabled: false
//                            border {
//                                color: "red"
//                                width: 1
//                            }
//                        }

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
        id: selectAttachmentDialog

        onAccepted: {
            var pos = selectAttachmentDialog.fileUrls.length - 1
            sendMessage(selectAttachmentDialog.fileUrls[pos], selectAttachmentDialog.attachmentType)
        }
    }

    function sendMessage(attachmentUrl, attachmentType) {
        const text = (messageField.text + messageField.preeditText).trim();
        messageField.clear()
        if (text || attachmentUrl)
            messageSending(text, attachmentUrl, attachmentType)
    }

    function selectAttachment(attachmentType) {
        selectAttachmentDialog.attachmentType = attachmentType
        selectAttachmentDialog.open()
    }

    Component.onCompleted: messageField.forceActiveFocus()
}
