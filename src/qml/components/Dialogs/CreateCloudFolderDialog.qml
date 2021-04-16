import QtQuick 2.15

import ".." // components

InputDialog {
    id: root
    title: qsTr("File Manager")
    label: qsTr("New directory")
    placeholderText: qsTr("Enter name")
    validator: app.validator.reDirectoryName

    signal newFolderRequested(string name)
    signal newSharedFolderRequested(string name)

    onAccepted: {
        const state = app.stateManager.cloudFileListState
        if (sharedCheckBox.checked) {
            root.newSharedFolderRequested(text)
        } else {
            root.newFolderRequested(text)
        }
    }
    onClosed: sharedCheckBox.checked = false

    StyledCheckBox {
        id: sharedCheckBox
        text: qsTr("Shared")
    }
}
