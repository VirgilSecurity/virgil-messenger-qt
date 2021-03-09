import QtQuick 2.15

import ".." // components

InputDialog {
    title: qsTr("File Manager")
    label: qsTr("New directory")
    placeholderText: qsTr("Enter name")
    validator: app.validator.reDirectoryName
    onAccepted: sharedCheckBox.checked ? appState.requestNewSharedFolder(text) : appState.requestNewFolder(text)
    onClosed: sharedCheckBox.checked = false

    StyledCheckBox {
        id: sharedCheckBox
        text: qsTr("Shared")
    }
}
