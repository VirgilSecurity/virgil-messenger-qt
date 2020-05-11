import QtQuick 2.12

import "../theme"
import "./CommonHelpers"

TextInCircle {
    property real count

    diameter: 16
    color: Theme.messegeCountColor
    opacity: count ? 1 : 0
    content: count > 9 ? '9+' : count
    pointSize: UiHelper.fixFontSz(9)
}
