import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

Column {
    id: root

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias horizontalAlignment: titleLabel.horizontalAlignment
    property alias bold: titleLabel.font.bold

    Label {
        id: titleLabel
        font.pointSize: UiHelper.fixFontSz(15)
        elide: Label.ElideRight
        color: Theme.primaryTextColor

        Binding on anchors.horizontalCenter { when: horizontalAlignment == Qt.AlignHCenter; value: parent.horizontalCenter }
        Binding on font.family { when: root.bold; value: Theme.mainFont }
    }

    Label {
        id: descriptionLabel
        font.pointSize: UiHelper.fixFontSz(11)
        elide: Label.ElideRight
        color: Theme.secondaryTextColor
        horizontalAlignment: titleLabel.horizontalAlignment

        Binding on anchors.horizontalCenter { when: horizontalAlignment == Qt.AlignHCenter; value: parent.horizontalCenter }
        Binding on font.family { when: root.bold; value: Theme.mainFont }
    }
}
