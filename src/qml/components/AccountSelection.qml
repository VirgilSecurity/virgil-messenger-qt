import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../theme"

ColumnLayout {
    id: accountSelector

    Layout.fillWidth: true
    Layout.maximumWidth: 350 // Theme.formMaximumWidth
    Layout.alignment: Qt.AlignHCenter

    spacing: Theme.spacing

    signal userSelected (string userName)

    Image {
        Layout.preferredHeight: 48
        Layout.preferredWidth: 48
        Layout.alignment: Qt.AlignHCenter

        fillMode: Image.PreserveAspectFit
        source: Theme.mainLogo
    }

    RowLayout {
        visible: true
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        spacing: 0

        Item {
            Layout.preferredHeight: 30
            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            ImageButton {
                image: "Arrow-Left"
                visible: view.currentIndex > 0
                onClicked: view.decrementCurrentIndex()
            }
        }


        SwipeView {
            id: view
            clip: true
            currentIndex: pageIndicator.currentIndex
            width: 240
            height: 240

            Repeater {
                model: models.accountSelection

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

                                onClicked: {
                                    accountSelector.userSelected(modelData)
                                }

                                background: Rectangle {
                                    color: "transparent"
                                    radius: 6
                                }

                                contentItem: Item {
                                    anchors.fill: parent

                                    Avatar {
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
                                        width: parent.width - Theme.margin
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

        Item {
            Layout.preferredHeight: 30
            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            ImageButton {
                image: "Arrow-Right"
                visible: view.currentIndex < view.count - 1
                onClicked: view.incrementCurrentIndex()
            }
        }
    }

    PageIndicator {
        id: pageIndicator
        interactive: true
        count: view.count
        currentIndex: view.currentIndex

        Layout.alignment: Qt.AlignHCenter
    }
}
