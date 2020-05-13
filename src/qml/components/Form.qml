import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Item {

    anchors.fill: parent

    property bool isBusy: false
    property alias busyDescription: busyDescryption.text

    default property alias children: formContainer.children

    ColumnLayout {
        id: formContainer
        visible: !isBusy

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
        }

        spacing: 15
    }

    // TODO: I don't like this eather, but let's
    // keep it simple for now ;)
    ColumnLayout {
        visible: isBusy

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
        }

        spacing: 15

        FormImage {
            source: "../resources/icons/Logo.png"
        }

        FormLabel {
            id: busyDescryption
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
