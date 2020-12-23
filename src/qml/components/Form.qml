import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    property bool isLoading: false
    property alias loadingText: busyDescryption.text

    default property alias children: formContainer.children
    property alias formSpacing: formContainer.spacing

    ColumnLayout {
        id: formContainer
        visible: !isLoading

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
        visible: isLoading

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
