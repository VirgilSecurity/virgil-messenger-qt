import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtMultimedia 5.15

import "../base"
import "../theme"
import "../components"

Page {
    id: chatPage

    readonly property var appState: app.stateManager.chatState
//    readonly property var chatName: controllers.chats.currentChatName
//    readonly property var contactId: controllers.chats.currentContactId
//    property real chatListViewHeight: 0

    property real chatListViewHeight: 0

    property bool notificationsEnabled: false
    property int menuBarIndex: 0

    QtObject {
        id: model
        readonly property string name: "Test chat name"
        readonly property string avatarUrl: ""
        readonly property string details: "some sort of details"
        readonly property real defaultChatHeight: 50

        readonly property string phone: "+380931234567"
        readonly property string contactId: "test_id"
    }

    QtObject {
        id: d
        readonly property real listSpacing: 5
        readonly property real smallItemHeight: 30
    }

    background: Rectangle {
        color: Theme.mainBackgroundColor
    }

    header: Control {
        id: headerControl
        width: parent.width
        height: Theme.headerHeight
        z: 1

        background: Rectangle {
            color: Theme.chatBackgroundColor
            anchors.leftMargin: 5
            anchors.rightMargin: 5
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }

        Column {
            anchors.fill: parent
            anchors.leftMargin: Theme.smallMargin
            anchors.rightMargin: Theme.smallMargin

            Row {
                width: parent.width
                height: parent.height
                spacing: Theme.smallSpacing

                ImageButton {
                    anchors.verticalCenter: parent.verticalCenter
                    image: "Arrow-Left"
    //                onClicked: app.stateManager.goBack()
                }

                Item {
                    id: avatarItem
                    anchors.verticalCenter: parent.verticalCenter
                    width: avatar.width + 8
                    height: width

                    Avatar {
                        id: avatar
                        nickname: model.name
                        avatarUrl: model.avatarUrl
                        anchors.centerIn: parent
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.name
                    }

                    Text {
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        text: model.details
                        width: parent.width
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                    }
                }
            }
        }
    }

    Flickable {
        width: parent.width
        height: parent.height
        contentHeight: commonColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: commonColumn
            spacing: Theme.spacing

            Column {
                id: column
                spacing: Theme.spacing

                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.margin
                }

                Item {
                    width: parent.width
                    height: 1
                }

                Text {
                    color: Theme.primaryTextColor
                    font.pointSize: UiHelper.fixFontSz(18)
                    text: qsTr("Information")
                }

                Column {

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.phone
                    }

                    Text {
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        text: qsTr("phone")
                        width: parent.width
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                    }
                }

                Column {

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.contactId
                    }

                    Text {
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        text: qsTr("id")
                        width: parent.width
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                    }
                }

                Column {

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: qsTr("Notifications")
                    }

                    Text {
                        color: Theme.secondaryTextColor
                        font.pointSize: UiHelper.fixFontSz(12)
                        text: notificationsEnabled ? qsTr("enabled") : qsTr("disabled")
                        width: parent.width
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                    }
                }
            }

            Item { // list view item
                id: listViewItem
                width: chatPage.width
                height: chatPage.height - Theme.headerHeight

                ListView {
                    id: menuBar
                    anchors {
                        left: parent.left
                        right: parent.right
                        leftMargin: Theme.margin
                        rightMargin: Theme.margin
                    }

                    height: d.smallItemHeight
                    orientation: Qt.Horizontal
                    model: menuModel
                    spacing: Theme.spacing * 1.5
                    currentIndex: menuBarIndex

                    ListModel {
                        id: menuModel

                        ListElement { name: qsTr("Participants") }
        //                ListElement { name: qsTr("Media") }
        //                ListElement { name: qsTr("Files") }
        //                ListElement { name: qsTr("Links") }
                    }

                    delegate: Item {
                        height: menuBar.height
                        width: menuBarText.width

                        Text {
                            id: menuBarText
                            height: parent.height
                            color: Theme.primaryTextColor
                            font.pointSize: UiHelper.fixFontSz(15)
                            text: name
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 3
                            radius: height
                            color: menuBarIndex === index ? Theme.contactPressedColor : "transparent"
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: cursorShape = Qt.PointingHandCursor
                            onExited: cursorShape = Qt.ArrowCursor
                            onClicked: menuBarIndex = index
                        }
                    }
                }

                SwipeView {
                    id: swipeView
                    currentIndex: menuBarIndex
                    onCurrentIndexChanged: menuBarIndex = currentIndex
                    clip: true
                    enabled: false

                    anchors {
                        top: menuBar.bottom
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                    }

                    Rectangle {
                        color: 'gray'
                        radius: 50
                    }
                }
            }
        }
    }

// Components
    Connections {
        target: appState

        function onMessageSent() {
            messageSent.play()
        }
    }
}

