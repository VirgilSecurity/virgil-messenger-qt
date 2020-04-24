import QtQuick 2.0

import "../../components"
import "../../theme"
import "../../js/strings.js" as Strings

TextInCircle {
    property string nickname

    color: Theme.avatarBgColor
    diameter: Theme.avatarHeight
    content: Strings.getPersonInitialis(nickname)
    pointSize: Theme.ff(20)
}
