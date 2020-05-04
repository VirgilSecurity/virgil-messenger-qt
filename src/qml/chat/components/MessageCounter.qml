import QtQuick 2.0

import "../../theme"
import "../../components"

TextInCircle {
    property real count

    diameter: 16
    color: Theme.messegeCountColor
    opacity: count ? 1 : 0
    content: count > 9 ? '9+' : count
    pointSize: UiHelper.fixFontSz(9)
}
