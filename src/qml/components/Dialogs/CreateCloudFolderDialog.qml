import QtQuick 2.15

import ".." // components

InputDialog {
    title: qsTr("File Manager")
    label: qsTr("New directory")
    placeholderText: qsTr("Enter name")
    validator: app.validator.reDirectoryName
    onAccepted: controllers.cloudFiles.createFolder(text)

    StyledCheckBox {
        text: qsTr("Shared")
    }
}
