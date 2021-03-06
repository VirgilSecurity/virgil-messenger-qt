import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../components"
import "../theme"

Page {
    background: Rectangle {
        color: Theme.contactsBackgroundColor
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SidebarPanel {
            Layout.preferredWidth: Theme.headerHeight
            Layout.fillHeight: true
        }

        MainListsPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
