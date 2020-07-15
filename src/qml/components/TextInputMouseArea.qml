import QtQuick 2.12
import Qt.labs.platform 1.0 as Native

MouseArea {
    id: root
    anchors.fill: textInput
    acceptedButtons: Qt.RightButton
    hoverEnabled: true

    property var textInput: parent

    Native.Menu {
        id: menu

        Native.MenuItem {
            text: qsTr("Cut")
            enabled: textInput.selectedText
            onTriggered: textInput.cut()
        }
        Native.MenuItem {
            text: qsTr("Copy")
            enabled: textInput.selectedText
            onTriggered: textInput.copy()
        }
        Native.MenuItem {
            text: qsTr("Paste")
            onTriggered: textInput.paste()
            // TODO(fpohtmeh): disable for empty clipboard
        }
    }

    function openMenu() {
        const selectionStart = textInput.selectionStart;
        const selectionEnd = textInput.selectionEnd;
        const cursorPosition = textInput.cursorPosition;
        menu.open();
        textInput.cursorPosition = cursorPosition;
        textInput.select(selectionStart, selectionEnd);
    }

    onClicked: openMenu()
    onPressAndHold: {
        if (mouse.source === Qt.MouseEventNotSynthesized)
            openMenu()
    }
}
