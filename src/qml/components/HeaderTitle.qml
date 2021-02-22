import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Column {
    id: root

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias horizontalAlignment: titleLabel.horizontalAlignment

    Label {
        id: titleLabel
        font.family: Theme.mainFont
        font.pointSize: UiHelper.fixFontSz(15)
        font.bold: true
        elide: Label.ElideRight
        color: Theme.primaryTextColor
        anchors.horizontalCenter: (horizontalAlignment == Qt.AlignHCenter) ? parent.horizontalCenter : undefined
    }

    Label {
        id: descriptionLabel
        font.family: Theme.mainFont
        font.pointSize: UiHelper.fixFontSz(11)
        elide: Label.ElideRight
        color: Theme.secondaryTextColor
        horizontalAlignment: titleLabel.horizontalAlignment
        anchors.horizontalCenter: (horizontalAlignment == Qt.AlignHCenter) ? parent.horizontalCenter : undefined
    }
}
