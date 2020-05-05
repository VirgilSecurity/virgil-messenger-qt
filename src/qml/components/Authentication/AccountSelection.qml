import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../../theme"
import "../ChatElements"
import "../../js/login.js" as LoginLogic

ColumnLayout {
    anchors.fill: parent

    Image {
        id: mainLogo
        Layout.preferredHeight: 48
        Layout.preferredWidth: 48
        Layout.maximumHeight: 48

        fillMode: Image.PreserveAspectFit
        source: Theme.mainLogo
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        Layout.bottomMargin: 10
    }

    Item {

        Layout.alignment: Qt.AlignCenter
        Layout.fillHeight: true
        Layout.fillWidth: true

        Button {
            background: Rectangle {
                color: "transparent"
            }

            contentItem: Item {
                Image  {
                    source: "../../resources/icons/Arrow-Left.png"
                    fillMode: Image.PreserveAspectFit
                    horizontalAlignment: Image.AlignRight
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            height: view.height
            width: 100
            anchors.top: view.top
            anchors.right: view.left
            visible: view.currentIndex > 0
            onPressed: view.decrementCurrentIndex()
        }

        Button {
            background: Rectangle {
                color: "transparent"
            }

            contentItem: Item {
                Image  {
                    source: "../../resources/icons/Arrow-Right.png"
                    fillMode: Image.PreserveAspectFit
                    horizontalAlignment: Image.AlignRight
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            height: view.height
            width: 100
            anchors.top: view.top
            anchors.left: view.right
            visible: view.currentIndex < view.count - 1
            onPressed: view.incrementCurrentIndex()
        }

        SwipeView {
            id: view

            clip: true
            currentIndex: pageIndicator.currentIndex

            height: 260
            width: 240

            anchors.centerIn: parent

            property var userChunks: LoginLogic.chunk(authenticationPage.userList, 4)

            Repeater {
                model: view.userChunks

                Item {
                    id: firstPage

                    Grid {
                        columns: 2
                        spacing: 2

                        Repeater {
                            model: modelData

                            Button {
                                width: 120
                                height: 120

                                onPressed: mainView.signIn(modelData)

                                background: Rectangle {
                                    color: "transparent"
                                    radius: 6
                                }

                                contentItem: Item {
                                    anchors.fill: parent

                                    AvatarPlaceholder {
                                        id: avatar
                                        nickname: modelData
                                        diameter: 60
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.top: parent.top
                                        anchors.topMargin: 15
                                    }

                                    Label {
                                        text: modelData
                                        anchors.top: avatar.bottom
                                        anchors.topMargin: 6
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        font.pointSize: UiHelper.fixFontSz(15)
                                        elide: Text.ElideRight
                                        width: parent.width - 20
                                        horizontalAlignment: Text.AlignHCenter
                                        color: Theme.primaryTextColor
                                    }
                                }

                            }
                        }
                    }
                }
            }

        }

        PageIndicator {
            id: pageIndicator
            interactive: true
            count: view.count
            currentIndex: view.currentIndex

            anchors.bottom: view.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}