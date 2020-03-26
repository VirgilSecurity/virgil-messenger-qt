import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "../helpers/ui"

Page {
    id: settingsPage

    property string userName: ""
    property string version: ""

    readonly property color kBtnColor: "#b44"

    background: Rectangle {
        color: backGroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 40

        Image {
            source: "qrc:/qml/resources/AppUserscreen.png"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.margins: 30
            text: qsTr("Account: ") + userName
            font.pointSize: 22
            color: mainTextCOlor
            elide: Text.ElideLeft
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.margins: 30
            text: qsTr("Version: ") + version
            font.pointSize: 22
            color: mainTextCOlor
            elide: Text.ElideLeft
        }

        CButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            height: 50

            name: qsTr("Logout")
            baseColor: kBtnColor
            borderColor: kBtnColor
            onClicked: {
                logout()
            }
        }

        Rectangle {
            height: 10
        }

        CButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            height: 50
            name: qsTr("Delete account")
            baseColor: kBtnColor
            borderColor: kBtnColor
            onClicked: {
                closeSettings()
            }
        }

        Rectangle {
            height: 10
            Layout.fillHeight: true
        }

        CButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            height: 50
            name: qsTr("Close")
            baseColor: "transparent"
            borderColor: kBtnColor
            onClicked: {
                closeSettings()
            }
        }
    }
}
