import QtQuick 2.0

import "../../theme"
import "../../components"

TextInCircle {
    property real count

    diameter: 16
    color: Theme.messegeCountColor

    content: count > 9 ? '9+' : count
    pointSize: 9
}
