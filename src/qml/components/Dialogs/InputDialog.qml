import QtQuick 2.15

import "../" // components

TemplateDialog {
    property alias label: formInput.label
    property alias text: formInput.text
    property alias placeholderText: formInput.placeholder
    acceptButtonEnabled: formInput.acceptableInput

    FormInput {
        id: formInput
        width: parent.width
        height: implicitHeight
        validator: app.validator.reDirectoryName
    }

    onRejected: text = ""
}
