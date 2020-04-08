import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../helpers/ui"
import "../theme"

Page {
    id: loginPage
    background: parent.background
    //
    //  Properties
    //
    property int operationTimeMaxMs: 1000

    //
    //  UI
    //

    ColumnLayout {
        width: parent.width
        spacing: 15


        Rectangle { Layout.minimumHeight: 50 }

        Image {
            id: iconRect
            width: parent.width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            fillMode: Image.PreserveAspectFit
            source: "../resources/AppUserscreen.png"
        }

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

        PrimaryButton {
            id: signUp
            width: parent.width
            height: 50
            name: qsTr("Sign Up")
            baseColor: "transparent"
            borderColor: mainAppColor
            Layout.fillWidth: true
            onClicked: signUpUser(loginUsername.editText)
        }

        OutlineButton {
            id: signIn
            width: parent.width
            name: qsTr("Sign in")
            onClicked: signInUser(loginUsername.editText)
            Layout.fillWidth: true
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
}
