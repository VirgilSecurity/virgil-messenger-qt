import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../helpers/ui"
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
                closeSettings()
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
                closeSettings()
            }
        }
    }
}
