import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

Column {
    Layout.fillWidth: true
    Layout.maximumWidth: Theme.formMaximumWidth
    Layout.alignment: Qt.AlignHCenter

    spacing: 5

    property alias label: formLabel.text
    property alias text: formField.text
    property alias placeholder: formField.placeholderText
    property alias inputHint: formField.inputMethodHints

    FormLabel {
        id: formLabel
    }

    UserNameTextField {
        id: formField
        width: parent.width
        height: 40
        leftPadding: 15
        rightPadding: 15
        font.pointSize: UiHelper.fixFontSz(15)
        color: Theme.primaryTextColor

        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            radius: 20
            color: Theme.inputBackgroundColor
        }
    }
}
