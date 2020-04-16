import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import "../helpers/ui"
import "../theme"

ScrollView {
    anchors.fill: parent
    clip: true
    contentHeight: Math.max(authenticationPage.currentItem.implicitHeight, rootWindow.height)

    StackView {
        id: authenticationPage
        initialItem: mainView

        anchors.fill: parent
        property var loginPage

        Component.onCompleted: {
            loginPage = Qt.createComponent("qrc:/qml/login/Login.qml")
            console.log()
        }

        Item {
            id: mainView
            anchors.fill: parent
            implicitHeight: 600

            ColumnLayout {
                anchors.fill: parent

                Item {
                    Layout.minimumHeight: 14
                    Layout.preferredHeight: 28
                    Layout.fillHeight: true
                }

                Item {
                    width: 300
                    Layout.minimumHeight: childrenRect.height
                    Layout.alignment: Qt.AlignCenter

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
                        onClicked: authenticationPage.push(authenticationPage.loginPage)
                        width: parent.width

                        anchors.topMargin: 15
                        anchors.top: registerButton.bottom
                    }

                }
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    Layout.topMargin: 15
                    Layout.bottomMargin: 15
                    text: qsTr("Powered by Virgil Security")
                    verticalAlignment: Text.AlignBottom
                    color: Theme.labelColor
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 11
                }
            }


        }

    }
}
