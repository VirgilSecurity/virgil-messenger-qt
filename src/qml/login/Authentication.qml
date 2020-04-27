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

    property var userList: Messenger.usersList()

    Component.onCompleted: {
        registerPage = Qt.createComponent("Register.qml")
        loginPage = Qt.createComponent("Login.qml")
    }

    Item {
        id: mainView

        CenteredAuthLayout {
            id: centeredAuthLayout

            content: ColumnLayout {
                spacing: 10
                Layout.maximumWidth: 300
                Layout.alignment: Qt.AlignCenter
                Layout.minimumHeight: 300
                Layout.maximumHeight: 480

                Loader {
                    Layout.minimumHeight: 220
                    Layout.maximumHeight: 420
                    Layout.alignment: Qt.AlignCenter
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    readonly property url mainLogoSrc: "./components/MainLogo.qml"
                    readonly property url userSelectionSrc: "./components/UserSelection.qml"

                    source: !userList.length ? mainLogoSrc : userSelectionSrc
                }

                Item {
                    Layout.maximumHeight: 90
                    Layout.minimumHeight: 0
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

