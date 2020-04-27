import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import "../../theme"
import "../../components"
import "../login.js" as LoginLogic

Item {
    anchors.fill: parent

    Image {
        id: mainLogo
        width: 48
        height: 48

        fillMode: Image.PreserveAspectFit
        source: Theme.mainLogo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
    }

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

        height: 240
        width: 240
        anchors.top: mainLogo.bottom
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter

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

                            onPressed: signInUser(modelData)

                            background: Rectangle {
                                color: "transparent"
                                radius: 6
                            }

                            contentItem: Item {
                                anchors.fill: parent;

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
        anchors.bottomMargin: -15
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
