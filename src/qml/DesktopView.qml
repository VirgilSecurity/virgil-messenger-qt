import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "chat"
import "login"

Rectangle {
    anchors.fill: parent
    color: "#000000"

    //
    //  Properties
    //
    property bool loginActive: true
    property alias chatView: chatView

    //
    //  Slots
    //
    onLoginActiveChanged: {
        // TODO: Activate chat
        if (!loginActive) {
        }
    }

    Login {
        id: login
        anchors.fill: parent
        visible: loginActive
    }


    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 7
        visible: !loginActive

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
