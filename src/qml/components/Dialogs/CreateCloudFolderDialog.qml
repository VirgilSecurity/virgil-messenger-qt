import QtQuick 2.15

import ".." // components

InputDialog {
    title: qsTr("File Manager")
    label: qsTr("New directory")
    placeholderText: qsTr("Enter name")
    validator: app.validator.reDirectoryName

    onAccepted: {
        const state = app.stateManager.cloudFileListState
        if (sharedCheckBox.checked) {
            state.requestNewSharedFolder(text)
        } else {
            state.requestNewFolder(text)
        }
    }
    onClosed: sharedCheckBox.checked = false

    StyledCheckBox {
        id: sharedCheckBox
        text: qsTr("Shared")
    }
}
