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
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 10
            Layout.minimumHeight: 40
            Layout.minimumWidth: 40

            onClicked: {
                showSettings()
            }

            background: Rectangle {
                color: "transparent"
            }

            Image {
                source: "qrc:/qml/resources/Settings.png"
                width: 30
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: settingsBtn.horizontalCenter
            }
        }
    }
}
