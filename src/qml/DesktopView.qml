import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.1

import "chat"
import "login"
import "settings"
import "theme"

Item {
    anchors.fill: parent

    //
    //  Properties
    //
    readonly property string kModeLogin: "login"
    readonly property string kModeSettings: "settings"
    readonly property string kModeNormal: "normal"

    property alias settings: settings

    property string mode: kModeLogin

    property alias chatView: chatView

    Authentication {
        id: authentication
        anchors.fill: parent
        visible: mode === kModeLogin
    }

    SettingsPage {
        id: settings
        anchors.fill: parent
        visible: mode === kModeSettings
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 7
        visible: mode === kModeNormal

        ContactPage {
            id: contactsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            Layout.maximumWidth: 300
        }

        ConversationPage {
            id: chatView
            Layout.fillWidth: true
            Layout.fillHeight: true
            bgColor: "#5D6D7E"
        }
    }
}
