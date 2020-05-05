import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../components/Buttons"
import "../theme"

Page {
    id: settingsPage

    background: Rectangle {
        color: backGroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40

        Image {
            source: "../resources/AppUserscreen.png"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.margins: 30
            text: qsTr("Account: ") + Messenger.currentUser
            font.pointSize: UiHelper.fixFontSz(22)
            color: mainTextCOlor
            elide: Text.ElideLeft
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.margins: 30
            text: qsTr("Version: ") + app.currentVersion()
            font.pointSize: UiHelper.fixFontSz(22)
            color: mainTextCOlor
            elide: Text.ElideLeft
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            name: qsTr("Check updates")
            onClicked: {
                app.checkUpdates()
            }
        }

        Rectangle {
            height: 10
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            name: qsTr("Send report to developers")
            onClicked: {
                close()
                app.sendReport()
            }
        }

        Rectangle {
            height: 10
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            name: qsTr("Logout")
            onClicked: {
                close()
                logout()
            }
        }

        Rectangle {
            height: 10
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            name: qsTr("Delete account")
            onClicked: {
                close()
            }
        }

        Rectangle {
            height: 10
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            name: qsTr("Close")
            onClicked: {
                close()
            }
        }
    }

    function close() {
        parent.pop(StackView.Immediate)
    }
}
