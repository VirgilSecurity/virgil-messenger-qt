import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../components/Buttons"
import "../templates"
import "../theme"

Page {
    id: authenticationPage

    property var userList: Messenger.usersList()
    property var loginPage
    property var registerPage

    background: Rectangle {
        color: "transparent"
    }

    header: ToolBar {
        implicitHeight: 60

        background: Rectangle {
            color: "transparent"
        }

        SettingsButton {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            Action {
                text: qsTr("Settings")
                onTriggered: mainView.push(Qt.createComponent('GlobalSettingsPage.qml'))
            }
        }
    }

    AuthenticationTemplate {
        id: centeredAuthLayout

        content: ColumnLayout {
            spacing: 10
            Layout.maximumWidth: 300
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumHeight: 260
            Layout.maximumHeight: 480

            Loader {
                Layout.minimumHeight: 220
                Layout.maximumHeight: 480
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: true
                readonly property url mainLogoSrc: "../components/Icons/MainLogo.qml"
                readonly property url userSelectionSrc: "../components/Authentication/AccountSelection.qml"

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
                onClicked: mainView.push(Qt.createComponent("RegisterPage.qml"))
            }

            OutlineButton {
                id: loginButton
                objectName: "bLogin"
                text: qsTr("Sign In")
                onClicked: mainView.push(Qt.createComponent("LoginPage.qml"))
                Layout.fillWidth: true
            }
        }
    }
}



