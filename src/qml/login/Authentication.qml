import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import "../helpers/ui"
import "../theme"
import "./components"

StackView {
    id: authenticationPage
    initialItem: mainView

    anchors.fill: parent
    property var loginPage
    property var registerPage

    Component.onCompleted: {
        registerPage = Qt.createComponent("qrc:/qml/login/Register.qml")
        loginPage = Qt.createComponent("qrc:/qml/login/Login.qml")
    }

    Item {
        id: mainView

        CenteredAuthLayout{
            id: centeredAuthLayout

            content: ColumnLayout {
                spacing: 10
                Layout.maximumWidth: 300
                Layout.alignment: Qt.AlignCenter
                Layout.minimumHeight: 400
                Layout.maximumHeight: 520

                Image {
                    id: mainLogo
                    width: 240
                    height: 240
                    fillMode: Image.PreserveAspectFit
                    source: Theme.mainLogo
                    Layout.preferredHeight: 240
                    Layout.alignment: Qt.AlignCenter
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
                    Layout.alignment: Qt.AlignCenter
                }

                Item {
                    Layout.maximumHeight: 90
                    Layout.minimumHeight: 10
                    Layout.fillHeight: true
                }

                PrimaryButton {
                    id: registerButton
                    text: qsTr("Register")
                    Layout.fillWidth: true
                    onClicked: authenticationPage.push(authenticationPage.registerPage)
                }

                OutlineButton {
                    id: loginButton
                    text: qsTr("Sign In")
                    onClicked: authenticationPage.push(authenticationPage.loginPage)
                    Layout.fillWidth: true
                }
            }
        }
    }
}

