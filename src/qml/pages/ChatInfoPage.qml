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

    property string chatName: qsTr("Chat info")
    property string chatDescription: ""
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
        readonly property real stdItemHeight: 40
        readonly property real bigItemHeight: 50

        readonly property real stdItemWidth: 280
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

            HorizontalRule {
                anchors.bottom: parent.bottom
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                color: Theme.chatSeparatorColor
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.smallMargin
            anchors.rightMargin: Theme.smallMargin

            ImageButton {
                image: "Arrow-Left"
//                onClicked: app.stateManager.goBack()
            }

            Column {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.smallMargin
                Label {
                    text: chatName
                    font.pointSize: UiHelper.fixFontSz(15)
                    color: Theme.primaryTextColor
                    font.bold: true
                }

                Label {
                    topPadding: 2
                    text: chatDescription
                    visible: chatDescription !== ""
                    font.pointSize: UiHelper.fixFontSz(12)
                    color: Theme.secondaryTextColor
                }
            }

            ImageButton {
                image: "More"
                onClicked: chatInfoContextMenu.open()

                ContextMenu {
                    id: chatInfoContextMenu
                    dropdown: true
                    currentIndex: -1

                    Action {
                        text: qsTr("Do something")
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

                Item { // spacing
                    width: parent.width
                    height: 1
                }

                Row {
                    spacing: Theme.smallSpacing

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

                Item { // spacing
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

            Column { // list view item
                id: listViewItem

                width: chatPage.width

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
                    interactive: false

                    ListModel {
                        id: menuModel

                        ListElement { name: qsTr("Participants") }
                        ListElement { name: qsTr("Media") }
                        ListElement { name: qsTr("Files") }
                        ListElement { name: qsTr("Links") }
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
                            onClicked: {
                                menuBarIndex = index
                            }
                        }
                    }
                }

                GridView {
                    id: participantsListView

                    QtObject {
                        id: gridProps
                        readonly property real cellWidth: calculateCellWidth()
                        readonly property real cellHeight: d.bigItemHeight
                        readonly property real targetCellWidth: d.stdItemWidth

                        function calculateCellWidth() {
                            if (!Platform.isDesktop) {
                                return chatPage.width
                            } else {
                                let numOfTargetCells = Math.round(chatPage.width / targetCellWidth)
                                return chatPage.width / numOfTargetCells
                            }
                        }
                    }

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    height: contentHeight
                    interactive: false

                    model: 30

                    cellWidth: gridProps.cellWidth
                    cellHeight: gridProps.cellHeight

                    footer: spacingDelegate
                    delegate: participantsDelegate
                    header: spacingDelegate

                    Component {
                        id: participantsDelegate

                        Item {
                            width: participantsListView.cellWidth
                            height: participantsListView.cellHeight

                            Row {
                                spacing: Theme.smallSpacing

                                anchors {
                                    fill: parent
                                    margins: Theme.margin
                                }

                                Item {
                                    id: avatarItem
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: avatar.width + 8
                                    height: width

                                    Avatar {
                                        id: avatar
                                        nickname: "model.name"
    //                                        avatarUrl: model.avatarUrl
                                        anchors.centerIn: parent
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        color: Theme.primaryTextColor
                                        font.pointSize: UiHelper.fixFontSz(15)
                                        text: "model.name " + index
                                    }

                                    Text {
                                        color: Theme.secondaryTextColor
                                        font.pointSize: UiHelper.fixFontSz(12)
                                        text: "model.details"
                                        width: parent.width
                                        elide: Text.ElideRight
                                        textFormat: Text.RichText
                                    }
                                }
                            }

                        }
                    }

                    Component {
                        id: spacingDelegate

                        Item {
                            width: parent.width
                            height: Theme.margin
                        }
                    }
                }


//                ListView {
//                    id: participantsListView

//                    anchors {
//                        left: parent.left
//                        right: parent.right
//                    }

//                    height: contentHeight
//                    spacing: Theme.minSpacing
//                    interactive: false

//                    model: 30

//                    footer: spacingDelegate
//                    delegate: participantsDelegate
//                    header: spacingDelegate

//                    Component {
//                        id: participantsDelegate

//                        Row {
//                            spacing: Theme.smallSpacing

//                            anchors {
//                                left: parent.left
//                                leftMargin: Theme.margin
//                            }

//                            Item {
//                                id: avatarItem
//                                anchors.verticalCenter: parent.verticalCenter
//                                width: avatar.width + 8
//                                height: width

//                                Avatar {
//                                    id: avatar
//                                    nickname: "model.name"
////                                        avatarUrl: model.avatarUrl
//                                    anchors.centerIn: parent
//                                }
//                            }

//                            Column {
//                                anchors.verticalCenter: parent.verticalCenter

//                                Text {
//                                    color: Theme.primaryTextColor
//                                    font.pointSize: UiHelper.fixFontSz(15)
//                                    text: "model.name " + index
//                                }

//                                Text {
//                                    color: Theme.secondaryTextColor
//                                    font.pointSize: UiHelper.fixFontSz(12)
//                                    text: "model.details"
//                                    width: parent.width
//                                    elide: Text.ElideRight
//                                    textFormat: Text.RichText
//                                }
//                            }
//                        }

//                    }

//                    Component {
//                        id: spacingDelegate

//                        Item {
//                            width: parent.width
//                            height: Theme.margin
//                        }
//                    }
//                }
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

