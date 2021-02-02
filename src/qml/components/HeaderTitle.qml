import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Column {
    anchors {
        verticalCenter: parent.verticalCenter
        left: parent.left
    }

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text

    Label {
        id: titleLabel
        elide: Label.ElideRight
        font.family: Theme.mainFont
        font.pointSize: UiHelper.fixFontSz(15)
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        font.bold: true
        color: Theme.primaryTextColor
    }

    Label {
        id: descriptionLabel
        elide: Label.ElideRight
        font.family: Theme.mainFont
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        font.pointSize: UiHelper.fixFontSz(11)
        color: Theme.secondaryTextColor
    }
}
