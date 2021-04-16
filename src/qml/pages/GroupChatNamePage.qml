import QtQuick 2.12

import "../components"

OperationPage {
    id: root

    signal named(string name)

    header: Header {
        showBackButton: !form.isLoading
        title: qsTr("New group")
    }

    Form {
        id: form

        FormInput {
            id: nameInput
            label: qsTr("Group name")
            placeholder: qsTr("Enter name")
            // TODO(fpohtmeh): validate
        }

        FormPrimaryButton {
            onClicked: root.named(nameInput.text)
            text: qsTr("Select members")
        }
    }
}
