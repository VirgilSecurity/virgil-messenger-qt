import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "../theme"

Label {
    property date date

    topPadding: Theme.smallPadding
    bottomPadding: Theme.smallPadding

    width: parent.width
    color: Theme.labelColor
    font.pointSize: UiHelper.fixFontSz(10)
    text: Qt.formatDate(date, "MMMM dd, yyyy")
    horizontalAlignment: Text.Center
}
