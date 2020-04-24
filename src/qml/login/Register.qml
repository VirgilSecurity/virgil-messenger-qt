import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../helpers/ui"
import "../theme"
import "./components"


Page {
    id: registerPage
    property real textSize: 12

    background: Rectangle {
        color: "transparent"
    }

    header: ToolBar {
        background: Rectangle {
            implicitHeight: 60
            color: "transparent"

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                height: 1
                color: Theme.toolbarSepratatorColor
                anchors.bottom: parent.bottom
            }
        }
        width: parent.width

        RowLayout {
            id: layout
            anchors.fill: parent

            ToolButton {
              background: Rectangle {
                  color: "transparent"
                  implicitHeight: 60
                  implicitWidth: 60
              }
              icon.source: "../resources/icons/Arrow-Left.svg"
              icon.height: 24
              icon.width: 24
              icon.color: Theme.secondaryTextColor
              onClicked: authenticationPage.pop()
              font.pointSize: Theme.ff(24)

            }

            Label {
              text: qsTr("Register")
              font.pointSize: Theme.ff(15)
              color: Theme.primaryTextColor
              elide: Label.ElideRight
              horizontalAlignment: Qt.AlignHCenter
              verticalAlignment: Qt.AlignVCenter
              Layout.fillWidth: true
            }

            Rectangle {
              color: "transparent"
              width: 60
              height: 60
            }
        }
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
                font.pointSize: Theme.ff(11)
            }

            TextField {
                id: username
                Layout.fillWidth: true
                Layout.topMargin: 10
                height: 40
                leftPadding: 15
                rightPadding: 15
                font.pointSize: Theme.ff(15)
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
                font.pointSize: Theme.ff(12)
                color: Theme.secondaryTextColor
                linkColor: Theme.buttonPrimaryColor
                wrapMode: Text.WordWrap
                text: "By creating an account, you agree to Virgil's <a href='https://virgilsecurity.com/terms-of-service/'>Terms of Service</a> and <a href='https://virgilsecurity.com/privacy-policy/'>Privacy Policy</a>"
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }
}
