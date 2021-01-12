import QtQuick 2.15
import QtQuick.Controls 2.15

import "../theme"

MenuSeparator {
    leftPadding: Theme.padding
    rightPadding: Theme.padding

    Binding on implicitHeight {
        when: !visible
        value: 0
    }
}
