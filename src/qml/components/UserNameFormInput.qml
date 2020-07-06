import QtQuick 2.12
import QtQuick.Controls 2.12
import "../theme"

FormInput {
    label: qsTr("Username")

    textField: UserNameTextField {
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            radius: 20
            color: Theme.inputBackgroundColor
        }
    }
}
