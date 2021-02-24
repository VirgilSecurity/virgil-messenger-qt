import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../theme"
import "../components"
import "../components/Dialogs"

Page {
    readonly property var appState: app.stateManager.cloudFileSharingState

    QtObject {
        id: d
        readonly property int selectedGroupMembersCount: 0
    }

    background: Rectangle {
        color: Theme.chatBackgroundColor
    }

    header: PageHeader {
        title: qsTr("Cloud folder info")
    }

    ColumnLayout {
        anchors {
            fill: parent
            leftMargin: Theme.smallMargin
            rightMargin: Theme.smallMargin
            topMargin: Theme.margin
            bottomMargin: Theme.margin
        }
        spacing: Theme.smallSpacing

        PropertiesView {
            Layout.leftMargin: Theme.smallMargin

            model: ListModel {
                ListElement {
                    propertyName: qsTr("Name:")
                    propertyValue: qsTr("Folder1")
                }
                ListElement {
                    propertyName: qsTr("Size:")
                    propertyValue: qsTr("1 Mb")
                }
            }
        }

        TabView {
            Layout.topMargin: Theme.margin
            Layout.fillHeight: true

            ContactsListView {
                readonly property var tabTitle: d.selectedGroupMembersCount
                                                ? qsTr("Participants (%1 selected)").arg(d.selectedGroupMembersCount)
                                                : qsTr("Participants")
            }
        }
    }
}
