import QtQuick 2.12
import QtQuick.Controls 2.12

Button {

    id: button

    property alias imageSource: button.icon.source

    icon.color: "transparent"
    padding: 0
    background: Rectangle {
        color: "transparent"
        implicitWidth: 24
        implicitHeight: 24
    }
}
