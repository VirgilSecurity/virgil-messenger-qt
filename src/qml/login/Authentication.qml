import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

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

    ColumnLayout {
        id: mainView
        width: 300
        anchors.horizontalCenter: parent.horizontalCenter

        spacing: 10

        Image {
            id: mainLogo
            width: 240
            height: 240
            fillMode: Image.PreserveAspectFit
            source: Theme.mainLogo
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            id: mainLogoText
            text: Theme.mainLogoText
            font.weight: Font.Bold
            font.capitalization: Font.AllUppercase

            font.family: Theme.mainFont
            font.pointSize: 48
            color: Theme.brandColor
            Layout.alignment: Qt.AlignCenter
            anchors.top: mainLogo.bottom
        }

        Text {
            id: mainLogoDescription
            width: 100
            text: qsTr("The most secure messenger on the market")
            lineHeight: 1
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.top: mainLogoText.bottom
            font.pointSize: 13
            color: Theme.secondaryFontColor
        }

        PrimaryButton {
            id: registerButton
            text: qsTr("Register")
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            anchors.top: mainLogoDescription.bottom
            anchors.topMargin: 90
        }

        OutlineButton {
            id: loginButton
            text: qsTr("Sign In")
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            anchors.top: registerButton.bottom
            anchors.topMargin: 15

            onClicked: authenticationPage.push(loginPage)
        }

        Text {
            text: qsTr("Powered by Virgil Security")
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            anchors.top: loginButton.bottom
            anchors.topMargin: 40
            color: Theme.labelColor
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 11
        }
    }
}
