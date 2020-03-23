import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "backend.js" as Backend
import "../helpers/ui"

Page {
    id: loginPage

    signal registerClicked()

    background: Rectangle {
        color: backGroundColor
    }

    ColumnLayout {
        width: parent.width
        height: parent.height

        spacing: 15

        Image {
            id: iconRect
            width: parent.width
            height: 50
            Layout.alignment: Qt.AlignHCenter

            fillMode: Image.PreserveAspectFit
            source: "../resources/AppUserscreen.png"
        }

        TextField {
            id: loginUsername
            placeholderText: qsTr("User name")
            Layout.preferredWidth: parent.width - 20
            Layout.alignment: Qt.AlignHCenter
            color: mainTextCOlor
            font.pointSize: 14
            leftPadding: 30
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 50
                radius: implicitHeight / 2
                color: "transparent"

                Image {
                    id: userImg
                    width: 30
                    height: 30

                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
//                    leftPadding: 10

                    fillMode: Image.PreserveAspectFit
                    source: "../resources/Contacts.png"
                }


                Rectangle {
                    width: parent.width - 10
                    height: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    color: mainAppColor
                }
            }
        }

        CButton {
            id: signIn
            height: 50
            Layout.preferredWidth: loginPage.width - 20
            Layout.alignment: Qt.AlignHCenter
            name: "Log In"
            baseColor: mainAppColor
            borderColor: mainAppColor
            onClicked: {
                loginUser(loginUsername.text)
            }
        }

        Rectangle {
            id: spacingR
            height: 10
            anchors.top: signIn.bottom
        }

        CButton {
            id: signUp
            height: 50
            anchors.top: spacingR.bottom
            Layout.preferredWidth: loginPage.width - 20
            Layout.alignment: Qt.AlignHCenter
            name: "Sign Up"
            baseColor: "transparent"
            borderColor: mainAppColor
            onClicked: stackView.push("qrc:/qml/start/SignUp.qml", {"uname": "arun", "pword": "some"}) //registerClicked()
        }

        Image {
            id: iconVirgil
            width: parent.width
            height: 100
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            fillMode: Image.PreserveAspectFit
            source: "../resources/PoweredBy.png"
        }

        Rectangle {
            height: 20
        }
    }
}
