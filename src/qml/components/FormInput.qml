import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../theme"

Column {
    id: root
    Layout.fillWidth: true
    Layout.maximumWidth: Theme.formMaximumWidth
    Layout.alignment: Qt.AlignHCenter
    spacing: 5

    property alias label: formLabel.text

    readonly property var formLabel: FormLabel {
        id: formLabel
    }
    property var textField: TextField {}

    children: [formLabel, textField]

    function updateTextField() {
        textField.implicitWidth = root.width
        textField.height = 40
        textField.leftPadding = 15
        textField.rightPadding = 15
        textField.font.pointSize = UiHelper.fixFontSz(15)
        textField.color = Theme.primaryTextColor
    }

    Component.onCompleted: updateTextField()
    onTextFieldChanged: updateTextField()
}
