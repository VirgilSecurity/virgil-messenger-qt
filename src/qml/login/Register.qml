import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../helpers/ui"
import "../theme"
import "./components"


Page {
    id: registerPage

    background: Rectangle {
        color: "transparent"
    }

    header: HeaderToolbar {
        title: qsTr("Register")
    }

    CenteredAuthLayout {
        content: ColumnLayout {

            Layout.preferredWidth: 260
            Layout.maximumWidth: 260
            Layout.alignment: Qt.AlignCenter

            spacing: 0

            Label {
                id: label
                Layout.fillWidth: true
                color: Theme.secondaryTextColor
                text: qsTr("Username")
                width: parent.width
                font.pointSize: UiHelper.fixFontSz(11)
            }

            TextField {
                id: username
                Layout.fillWidth: true
                Layout.topMargin: 10
                height: 40
                leftPadding: 15
                rightPadding: 15
                font.pointSize: UiHelper.fixFontSz(15)
                color: Theme.primaryTextColor

                background: Rectangle {
                    implicitWidth: parent.width
                    implicitHeight: parent.height
                    radius: 20
                    color: Theme.inputBackgroundColor
                }
            }

            PrimaryButton {
                text: qsTr("Create account")
                onClicked: signUpUser(username.text)
                Layout.fillWidth: true
                Layout.topMargin: 15
            }

            Text {
                Layout.topMargin: 10
                Layout.maximumWidth: parent.width
                font.pointSize: UiHelper.fixFontSz(12)
                color: Theme.secondaryTextColor
                linkColor: Theme.buttonPrimaryColor
                wrapMode: Text.WordWrap
                text: "By creating an account, you agree to Virgil's <a href='https://virgilsecurity.com/terms-of-service/'>Terms of Service</a> and <a href='https://virgilsecurity.com/privacy-policy/'>Privacy Policy</a>"
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }
}
