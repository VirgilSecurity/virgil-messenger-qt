import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.platform 1.0 as Native
import com.virgilsecurity.messenger 1.0

import "../base"
import "../theme"

Control {
    id: root

    width: parent.width
    implicitHeight: row.height

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    RowLayout {
        id: row
        anchors.left: parent.left
        anchors.right: parent.right

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
                    onTriggered: attachmentPicker.open(Enums.AttachmentType.Picture)
                }

                Action {
                    text: qsTr("Send file")
                    onTriggered: attachmentPicker.open(Enums.AttachmentType.File)
                }
            }
        }

        Rectangle {
            id: inputRect
            Layout.fillWidth: true
            Layout.preferredHeight: scrollView.height + Theme.smallMargin
            Layout.topMargin: Theme.smallMargin
            Layout.bottomMargin: Theme.smallMargin
            radius: 20
            color: "#37474F"

            TextScrollView {
                id: scrollView
                height: Math.min(100, messageField.implicitHeight)
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 0.5 * Theme.smallMargin
                    leftMargin: Theme.smallMargin
                }

                TextArea {
                    id: messageField
                    placeholderText: qsTr("Message")
                    placeholderTextColor: "#59717D"
                    wrapMode: TextArea.Wrap
                    font.pixelSize: 15
                    color: Theme.primaryTextColor
                    verticalAlignment: TextEdit.AlignVCenter
                    selectByMouse: Platform.isDesktop
                    selectedTextColor: "black"
                    selectionColor: "white"

                    readonly property int maximumLength: 4096
                    property string previousText: text

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

                    onTextChanged: {
                        if (text.length > maximumLength) {
                            stateSaver(function() {
                                text = previousText
                                showPopupInform("Message length limit: %1".arg(maximumLength))
                            }, previousText.length - text.length)
                        }
                        previousText = text
                    }

                    function stateSaver(func, offset=0) {
                        const selStart = selectionStart + offset
                        const selEnd = selectionEnd + offset
                        const curPos = cursorPosition + offset
                        var scroll = scrollView.ScrollBar.vertical
                        const scrollPos = scroll.position
                        func()
                        messageField.cursorPosition = curPos
                        messageField.select(selStart, selEnd)
                        scroll.position = scrollPos
                    }

                    function openContextMenu() {
                        stateSaver(contextMenu.open)
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: messageField.openContextMenu()
                onPressAndHold: {
                    if (mouse.source === Qt.MouseEventNotSynthesized) {
                        messageField.openContextMenu()
                    }
                }

                Native.Menu {
                    id: contextMenu
                    Native.MenuItem {
                        text: qsTr("Cut")
                        onTriggered: messageField.cut()
                    }
                    Native.MenuItem {
                        text: qsTr("Copy")
                        onTriggered: messageField.copy()
                    }
                    Native.MenuItem {
                        text: qsTr("Paste")
                        onTriggered: messageField.paste()
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

    function sendMessage(attachmentUrl, attachmentType) {
        const text = (messageField.text + messageField.preeditText).trim();
        messageField.clear()
        controllers.messages.createSendMessage(text, attachmentUrl, attachmentType)
    }

    Connections {
        target: attachmentPicker

        function onPicked(fileUrls, attachmentType) {
            if (manager.currentState !== manager.chatState) {
                return;
            }
            const url = fileUrls[fileUrls.length - 1]
            sendMessage(url, attachmentType)
        }
    }

    Component.onCompleted: {
        if (Platform.isDesktop) {
            messageField.forceActiveFocus();
        }
        else if (Platform.isIos) {
            app.keyboardEventFilter.install(messageField)
        }
    }
}
