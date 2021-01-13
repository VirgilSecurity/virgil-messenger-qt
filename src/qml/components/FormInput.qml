import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../base"
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
    property alias validator: formField.validator
    property alias acceptableInput: formField.acceptableInput
    property bool password: false

    FormLabel {
        id: formLabel
        width: parent.width
    }

    TextField {
        id: formField
        width: parent.width
        height: 40
        leftPadding: 15
        rightPadding: 15
        font.pointSize: UiHelper.fixFontSz(15)
        color: Theme.primaryTextColor
        echoMode: password ? TextField.Password : TextField.Normal
        focus: Platform.isDesktop

        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            radius: 20
            color: Theme.inputBackgroundColor
        }
    }
}
