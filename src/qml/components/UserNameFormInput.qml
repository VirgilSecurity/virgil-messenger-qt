import QtQuick 2.12
import QtQuick.Controls 2.12
import "../theme"

FormInput {
    label: qsTr("Username")
    textField: UserNameTextField {
        placeholderText: qsTr("Enter Username")

        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            radius: 20
            color: Theme.inputBackgroundColor
        }
    }
}
