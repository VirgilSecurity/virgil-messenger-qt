import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import "../base"
import "../components"
import "../theme"

OperationPage {
    appState: app.stateManager.newGroupChatState
    loadingText: qsTr("Adding of contact...")
    footerText: ""
    readonly property string contact: contactSearch.search.toLowerCase()
    readonly property var filterSource: models.discoveredContacts
    readonly property alias search: contactSearch.search
    property string previousSearch
    readonly property int modelCount: searchResultsItem.modelCount
    property string serverName: "Default"

    readonly property int defaultSearchHeight: 40
    readonly property int defaultChatHeight: 50
    readonly property int flowSpaing: 3
    readonly property int flowItemHeight: 30
    readonly property int lineWidth: 2


    onSearchChanged: {
        if (search) {
            previousSearch = search
        }
        if (filterSource) {
            filterSource.filter = search
        }
    }

    header: Header {
        title: qsTr("New group")
    }

    Item {
        id: addedContacts
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: Theme.margin
            rightMargin: Theme.margin
        }
        height: {
            if (addedContactsView.contentHeight > defaultChatHeight * 2) {
                return defaultChatHeight * 2
            } else {
                return addedContactsView.contentHeight
            }
        }

        Behavior on height {
            NumberAnimation {
                easing.type: Easing.InOutCubic
                duration: Theme.animationDuration
            }
        }

        FlowListView {
            id: addedContactsView
            anchors.fill: parent
            model: flowModel
            spacing: flowSpaing
            delegate: addedContactComponent
            focus: true
            onFocusChanged: {
                if (!focus) {
                    addedContactsView.currentIndex = -1
                }
            }
        }

        Component {
            id: addedContactComponent
            Rectangle {
                id: contactRec
                height: avatar.height
                width: row.width + row.spacing
                color: addedContactsView.currentIndex === index ? Theme.buttonPrimaryColor : Theme.contactPressedColor
                radius: height

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        addedContactsView.focus = true
                        if (addedContactsView.currentIndex !== index) {
                            addedContactsView.currentIndex = index
                        } else {
                            addedContactsView.currentIndex = -1
                        }
                    }
                }

                Row {
                    id: row
                    spacing: Theme.smallSpacing
                    anchors.verticalCenter: parent.verticalCenter
                    Item {
                        height: flowItemHeight
                        width: height
                        visible: addedContactsView.currentIndex === index
                        enabled: addedContactsView.currentIndex === index
                        Repeater {
                            model: 2
                            Rectangle {
                                anchors.centerIn: parent
                                width: 0.5 * parent.width
                                height: lineWidth
                                radius: height
                                color: Theme.brandColor
                                rotation: index ? -45 : 45
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                addedContactsView.currentIndex = -1
                                addedContactsView.model.remove(index)
                            }
                        }
                    }

                    Avatar {
                        id: avatar
                        nickname: model.name
                        avatarUrl: model.avatarUrl
                        diameter: flowItemHeight
                        anchors.verticalCenter: parent.verticalCenter
                        visible: addedContactsView.currentIndex !== index
                    }

                    Text {
                        color: Theme.primaryTextColor
                        font.pointSize: UiHelper.fixFontSz(15)
                        text: model.name
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

    }

    Search {
        id: contactSearch
        anchors {
            top: addedContacts.bottom
            left: parent.left
            right: parent.right
            margins: Theme.margin
            topMargin: 0
        }
        height: defaultSearchHeight
        state: "opened"
        searchPlaceholder: qsTr("Search contact")
        onClosed: {
            contactSearch.search = ""
            reject()
        }
        onAccepted: {
            accept()
        }
    }

    SelectContactsList {
        id: searchResultsItem
        anchors {
            top: contactSearch.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            topMargin: 1
            bottomMargin: defaultChatHeight
        }
//        model: models.discoveredContacts.proxy
        model: contactsModel
        multiselect: true
    }

    RowLayout {
        anchors {
            top: searchResultsItem.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        Label {
            text: qsTr("Server")
            color: Theme.primaryTextColor
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            text: serverName
            color: Theme.secondaryTextColor
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }
    }

    function selectContact(index) {
        console.log("selectContact:", index)
        let item = contactsModel.get(index)
        if (item['selected'] === false) {
            item['selected'] = true
            flowModel.append(item)
            console.log("flowModel.append:", index)
        } else {
            item['selected'] = false
            console.log("item['selected'] = false:", index)

            let i
            for (i = 0; i < flowModel.count; i++) {
                let flowFalseItem = flowModel.get(i).selected
                if (flowFalseItem === false) {
                    flowModel.remove(i)
                    console.log("flowModel.remove:", index)
                }
            }
        }
    }

    function accept() {
    }

    function reject() {
        app.stateManager.goBack()
    }

    // TEMP

    ListModel {
        id: contactsModel

        ListElement {
            name: "John Doe"
            avatarUrl: "https://avatars.mds.yandex.net/get-zen_doc/1779726/pub_5d32ac8bf2df2500adb00103_5d32aeae21f9ff00ad9973ee/scale_1200"
            lastSeenActivity: "yesterday"
            selected: false
        }
        ListElement {
            name: "Bon Min"
            avatarUrl: "https://peopletalk.ru/wp-content/uploads/2016/10/orig_95f063cefa53daf194fa9f6d5e20b86c.jpg"
            lastSeenActivity: "yesterday"
            selected: false
        }
        ListElement {
            name: "Tin Bin"
            avatarUrl: "https://i.postimg.cc/wBJKr6CR/K5-W-z1n-Lqms.jpg"
            lastSeenActivity: "yesterday"
            selected: false
        }
        ListElement {
            name: "Mister Bean"
            avatarUrl: "https://avatars.mds.yandex.net/get-zen_doc/175962/pub_5a7b1334799d9dbfb9cc0f46_5a7b135b57906a1b6eb710eb/scale_1200"
            lastSeenActivity: "yesterday"
            selected: false
        }
        ListElement {
            name: "Erick Helicopter"
            avatarUrl: ""
            lastSeenActivity: "yesterday"
            selected: false
        }
        ListElement {
            name: "Peter Griffin"
            avatarUrl: ""
            lastSeenActivity: "yesterday"
            selected: false
        }
    }

    ListModel {
        id: flowModel
    }

}
