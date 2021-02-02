import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.0 as Native

import "../base"
import "../theme"

Control {
    id: root

    width: parent.width
    implicitHeight: row.height

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    QtObject {
        id: d

        readonly property string messageText: (messageField.text + messageField.preeditText).trim()

        function sendTextMessage() {
            const text = d.messageText;
            if (text) {
                messageField.clear()
                controllers.messages.sendTextMessage(text)
            }
        }

        function sendFileMessage(attachmentUrl, attachmentType) {
            if (attachmentType === AttachmentTypes.picture) {
                controllers.messages.sendPictureMessage(attachmentUrl, attachmentType)
            }
            else {
                controllers.messages.sendFileMessage(attachmentUrl, attachmentType)
            }
        }
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
                    onTriggered: attachmentPicker.open(AttachmentTypes.picture)
                }

                Action {
                    text: qsTr("Send file")
                    onTriggered: attachmentPicker.open(AttachmentTypes.file)
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
            color: Theme.inputBackgroundColor

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
                    placeholderTextColor: "#59717D" // TODO(fpohtmeh): move this to Theme
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
                                d.sendTextMessage()
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
            disabled: d.messageText.length == 0
            image: "Send"
            onClicked: d.sendTextMessage()
        }
    }

    Connections {
        target: attachmentPicker

        function onPicked(fileUrls, attachmentType) {
            if (manager.currentState !== manager.chatState) {
                return;
            }
            const url = fileUrls[fileUrls.length - 1]
            d.sendFileMessage(url, attachmentType)
        }
    }

    Component.onCompleted: {
        if (Platform.isDesktop) {
            messageField.forceActiveFocus();
        } else if (Platform.isIos) {
            app.keyboardEventFilter.install(messageField)
        }
    }
}
