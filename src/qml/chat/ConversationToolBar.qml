import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

ToolBar {
    spacing: 20
    height: toolbarHeight

    background: Rectangle {
        color: toolbarColor
    }

    RowLayout {
        anchors.fill: parent

        ToolButton {
            id: settingsBtn
            visible: isMobileView()
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.leftMargin: 10
            Layout.minimumHeight: 40
            Layout.minimumWidth: 40

            onClicked: {
                chatWorkspaceStack.showContacts()
            }

            background: Rectangle {
                color: "transparent"
            }

            Image {
                source: "../resources/AppUserscreen.png"
                width: 30
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: settingsBtn.horizontalCenter
            }
        }
        Label {
            text: inConversationWith
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
            color: toolbarTextColor
            font.bold: true
        }
    }
}
