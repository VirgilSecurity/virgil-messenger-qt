import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "login.js" as LoginLogic
import "../helpers/ui"

Page {
    id: loginPage
    objectName: "LoginObject"

    //
    // Signals
    //
    signal fireSignIn(string user)
    signal fireSignUp(string user)

    //
    // Properties
    //
    property int operationTimeMaxMs: 3000


    //
    //  UI
    //
    background: Rectangle {
        color: backGroundColor
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 15

        Rectangle { Layout.minimumHeight: 50 }

        Image {
            id: iconRect
            width: parent.width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            fillMode: Image.PreserveAspectFit
            source: "../resources/AppUserscreen.png"
        }

        Item {
            id: buttonsLogin
            Layout.preferredWidth: loginPage.width - 20
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true

            TextField {
                id: loginUsername
                placeholderText: qsTr("User name")

                width: parent.width
                color: mainTextCOlor
                leftPadding: 40

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

            Rectangle {
                id: spacingR0
                height: 10
                anchors.top: loginUsername.bottom
            }

            CButton {
                id: signIn
                width: parent.width
                height: 50
                anchors.top: spacingR0.bottom

                name: qsTr("Log In")
                baseColor: mainAppColor
                borderColor: mainAppColor
                onClicked: signInUser(loginUsername.text)
            }

            Rectangle {
                id: spacingR
                height: 10
                anchors.top: signIn.bottom
            }

            CButton {
                id: signUp
                width: parent.width
                height: 50
                anchors.top: spacingR.bottom
                name: qsTr("Sign Up")
                baseColor: "transparent"
                borderColor: mainAppColor
                onClicked: signUpUser(loginUsername.text)
            }
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
            height: 1
        }
    }

    //
    //  Functions
    //

    function showProgress(title) {
        stackView.push("qrc:/qml/helpers/ui/Progress.qml", {"titleStr": title, "reqTimeMs": operationTimeMaxMs, "timerStart": true})
        stackView.currentItem.done.connect(function() {
            stackView.push("qrc:/qml/chat/ContactPage.qml")
        })
    }

    function signInUser(user) {
        if (LoginLogic.validateUser(user)) {
            showProgress(qsTr("Sign In"))
            loginPage.fireSignIn(user)
        } else {
            showPopupMessage(qsTr("Incorrect user name"))
        }
    }

    function signUpUser(user) {
        if (LoginLogic.validateUser(user)) {
            showProgress(qsTr("Sign Up"))
            loginPage.fireSignUp(user)
        } else {
            showPopupMessage(qsTr("Incorrect user name"))
        }
    }
}
