import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"

GridLayout {
    columns: 2
    columnSpacing: Theme.spacing

    property alias model: repeater.model

    Repeater {
        id: repeater

        Repeater {
            model: 2
            delegate: FormLabel {
                readonly property bool isName: model.index % 2 == 0
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: !isName
                color: isName ? Theme.labelColor : Theme.primaryTextColor
                text: (isName ? propertyName : propertyValue)
                font.bold: isName
            }
        }
    }
}
