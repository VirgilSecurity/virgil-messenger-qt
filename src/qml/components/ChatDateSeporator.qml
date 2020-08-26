import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Label {
    property date date

    topPadding: 10
    bottomPadding: 10

    width: parent.width
    color: Theme.labelColor
    font.pointSize: UiHelper.fixFontSz(10)
    text: Qt.formatDate(date, "MMMM dd, yyyy")
    horizontalAlignment: Text.Center
}
