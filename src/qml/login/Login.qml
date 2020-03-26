import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "login.js" as LoginLogic
import "../helpers/ui"

Page {
    id: loginPage
    objectName: "LoginObject"

    //
    //  Properties
    //
    property int operationTimeMaxMs: 1000

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

            ComboBox {
                id: loginUsername
                width: parent.width
                leftPadding: 10
                rightPadding: 30

                editable: true
                flat: true
                model: Messenger.usersList()

                contentItem: TextField {
                    placeholderText: qsTr("User name")
                    text: loginUsername.displayText

                    color: mainTextCOlor
                    leftPadding: 40

                    background: Rectangle {
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
                    }
                }

                indicator: Canvas {
                    id: canvas
                    x: loginUsername.width - width - 10
                    y: loginUsername.topPadding + (loginUsername.availableHeight - height) / 2
                    width: 15
                    height: 15
                    contextType: "2d"

                    Connections {
                        target: loginUsername
                        onPressedChanged: canvas.requestPaint()
                    }

                    onPaint: {
                        context.reset();
                        context.moveTo(0, 0);
                        context.lineTo(width, 0);
                        context.lineTo(width / 2, height);
                        context.closePath();
                        context.fillStyle = loginUsername.pressed ? "grey" : mainTextCOlor;
                        context.fill();
                    }
                }

                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 50
                    radius: implicitHeight / 2
                    color: "transparent"

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
                onClicked: signInUser(loginUsername.editText)
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
                onClicked: signUpUser(loginUsername.editText)
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

    // Show progress page
    function showProgress(title) {
        mobileView.push("qrc:/qml/helpers/ui/Progress.qml", {"titleStr": title, "reqTimeMs": operationTimeMaxMs, "timerStart": true})
    }

    function signInUser(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signIn(user)
            showPopupInform("Sign In ...")
            mobileView.push("qrc:/qml/chat/ContactPage.qml")
            desktopView.loginActive = false
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }

    function signUpUser(user) {
        if (LoginLogic.validateUser(user)) {
            Messenger.signUp(user)
            showPopupInform("Sign Up ...")
            mobileView.push("qrc:/qml/chat/ContactPage.qml")
            desktopView.loginActive = false
        } else {
            showPopupError(qsTr("Incorrect user name"))
        }
    }
}
