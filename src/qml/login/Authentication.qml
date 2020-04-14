import QtQuick 2.7
import QtQuick.Controls 2.4

import "../helpers/ui"
import "../theme"

StackView {

    id: authenticationPage

    initialItem: mainView
    anchors.fill: parent
    width: parent.width

    property var loginPage

    Component.onCompleted: {
        loginPage = Qt.createComponent("qrc:/qml/login/Login.qml")
    }

    Item {
        id: mainView
        width: 300
        height: 600
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.topMargin: 20
        anchors.bottomMargin: 20

        Image {
            id: mainLogo
            width: 240
            height: 240
            fillMode: Image.PreserveAspectFit
            source: Theme.mainLogo
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: mainLogoText
            text: Theme.mainLogoText
            font.weight: Font.Bold
            font.capitalization: Font.AllUppercase
            font.family: Theme.mainFont
            font.pointSize: 48
            color: Theme.brandColor

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: mainLogo.bottom
        }

        Text {
            id: mainLogoDescription
            width: 180
            text: qsTr("The most secure messenger on the market")
            lineHeight: 1
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter

            font.pointSize: 13
            color: Theme.secondaryTextColor

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: mainLogoText.bottom
        }

        PrimaryButton {
            id: registerButton
            text: qsTr("Register")
            width: parent.width
            anchors.topMargin: 90
            anchors.top: mainLogoDescription.bottom

        }

        OutlineButton {
            id: loginButton
            text: qsTr("Sign In")
            onClicked: authenticationPage.push(loginPage)
            width: parent.width

            anchors.topMargin: 15
            anchors.top: registerButton.bottom
        }

        Text {
            text: qsTr("Powered by Virgil Security")
            color: Theme.labelColor
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 11

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 40
            anchors.top: loginButton.bottom
        }
    }
}
