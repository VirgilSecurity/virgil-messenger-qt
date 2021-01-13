import QtQuick 2.15
import QtQuick.Controls 2.15
import Customers 1.0

Rectangle {
    id: button

    readonly property int defaultMargin: 9
    readonly property int size: 36
    property int imageSize: 28
    property alias imageSource: image.source

    signal clicked

    implicitHeight: size
    implicitWidth: size
    color: Customer.serverIconBackgroundColor
    radius: defaultMargin

    Image {
        id: image
        anchors.centerIn: parent
        width: imageSize
        height: imageSize
        source: "../resources/icons/Logo.png"
        fillMode: Image.PreserveAspectFit
    }

    MouseArea {
        anchors.fill: parent
        onClicked: button.clicked()
    }
}
