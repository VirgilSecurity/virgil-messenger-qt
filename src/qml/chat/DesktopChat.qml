import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.1

RowLayout {
    id: desktopLayout
    anchors.fill: parent
    spacing: 7

    property alias chatView: chatView

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
