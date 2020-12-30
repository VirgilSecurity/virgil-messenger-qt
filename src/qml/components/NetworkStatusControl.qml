import QtQuick.Controls 2.15

Label {
    color: "red"
    text: qsTr("Connecting...")
    font.pointSize: UiHelper.fixFontSz(32)
    visible: settings.devMode && !messenger.isOnline && controllers.users.currentUserId
}
