import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"

Item {
    id: form
    anchors.fill: parent

    default property alias children: formContainer.children
    property bool isLoading: false
    property bool isCentered: true

    ColumnLayout {
        id: formContainer
        visible: !isLoading
        spacing: Theme.spacing

        anchors {
            verticalCenter: isCentered ? parent.verticalCenter : undefined
            left: parent.left
            right: parent.right
            top: isCentered ? undefined : parent.top
            bottom: isCentered ? undefined : parent.bottom
            leftMargin: isCentered ? undefined : Theme.margin
            rightMargin: isCentered ? undefined : Theme.margin
            bottomMargin: isCentered ? undefined : Theme.margin
            topMargin: isCentered ? undefined : Theme.margin
        }
    }

    ColumnLayout {
        visible: isLoading
        spacing: Theme.spacing

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
        }

        FormImage {
            source: "../resources/icons/Logo.png"
        }

        FormLabel {
            id: busyDescryption
            horizontalAlignment: Text.AlignHCenter
        }
    }

    function showLoading(loadingText) {
        isLoading = true
        busyDescryption.text = loadingText
    }

    function hideLoading() {
        isLoading = false
        busyDescryption.text = ""
    }
}
