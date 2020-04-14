import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.1

import "../theme"

ToolBar {
    spacing: 20
    height: 60

    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    Item {
        id: container
        anchors.leftMargin: 20
        anchors.rightMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 15
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Column {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("Server Name")
                        font.pointSize: 15
                        color: Theme.primaryTextColor
                        font.bold: true
                    }

                    Label {
                        text: qsTr("Server description")
                        font.pointSize: 12
                        color: Theme.secondaryFontColor
                    }
                }

                ToolButton {
                    id: settingsBtn
                    Layout.alignment: Qt.AlignRight
                    Layout.minimumHeight: 40
                    Layout.minimumWidth: 40
                    background: Rectangle {
                        color: "transparent"
                    }

                    onClicked: {
                        addContact()
                    }

                    Image {
                        source: "qrc:/qml/resources/icons/Search.svg"
                        width: 24
                        height: 24
                        smooth: true
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: settingsBtn.horizontalCenter
                    }
                }

                ToolButton {
                    id: control
                    text: qsTr("⋮")

                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {
                        menu.x = control.width - menu.width
                        menu.y = control.height + container.anchors.bottomMargin

                        menu.open()
                    }
                    background: Rectangle {
                        color: "transparent"
                    }

                    contentItem: Text {
                         text: control.text
                         color: Theme.primaryTextColor
                         font.pointSize: 24
                    }

                    Menu {
                        property real menuWidth: 300

                        id: menu

                        Action { text: qsTr("New chat") }

                        MenuSeparator { }

                        Action { text: qsTr("Add to contacts") }

                        background: Rectangle {
                            implicitWidth: 200
                            implicitHeight: 200
                            color: Theme.menuBackgroundColor
                            radius: 6
                        }
                    }
                }
            }


            ToolSeparator {
                id: separator
                orientation: Qt.Horizontal
                Layout.fillWidth: true

                contentItem: Rectangle {
                    implicitHeight: 1
                    width: parent.width
                    color: Theme.sepratorColor
                }
            }
        }
    }


}
