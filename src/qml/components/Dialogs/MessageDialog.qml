import QtQuick 2.15

import "../../theme"

StyledDialog {
    property alias text: textLabel.text

    Text {
        id: textLabel
        width: parent.width
        color: Theme.primaryTextColor
        font.pointSize: UiHelper.fixFontSz(12)
        wrapMode: Text.WordWrap
    }
}
