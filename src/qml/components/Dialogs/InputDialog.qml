import QtQuick 2.15

import "../" // components

StyledDialog {
    property alias label: formInput.label
    property alias text: formInput.text
    property alias placeholderText: formInput.placeholder
    property alias validator: formInput.validator
    acceptButtonEnabled: formInput.acceptableInput

    FormInput {
        id: formInput
        width: parent.width
        height: implicitHeight
    }

    onClosed: text = ""
}
