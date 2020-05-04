import QtQuick 2.12

import "../../theme"
import "../CommonHelpers"
import "../../js/chat.js" as ChatLogic

TextInCircle {
    property string nickname

    color: Theme.avatarBgColor
    diameter: Theme.avatarHeight
    content: ChatLogic.getPersonInitialis(nickname)
    pointSize: UiHelper.fixFontSz(20)
}
