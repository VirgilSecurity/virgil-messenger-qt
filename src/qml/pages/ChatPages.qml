import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../base"
import "../components"
import "../theme"

Item {
    id: root

    signal newChatRequested()
    signal newGroupChatRequested()
    signal infoRequested()

    QtObject {
        id: d

        readonly property var lm: Theme.landscapeMode
        readonly property bool isLandscapeMode: Platform.isDesktop && width >= lm.listMinimumWidth + lm.splitHandleSize + lm.chatMinimumWidth
    }

    Component {
        id: splitViewComponent
        ChatPagesSplitView {
            anchors.fill: parent

            onNewChatRequested: root.newChatRequested()
            onNewGroupChatRequested: root.newGroupChatRequested()
            onInfoRequested: root.infoRequested()
        }
    }

    Component {
        id: stackViewComponent
        ChatPagesStackView {
            anchors.fill: parent

            onNewChatRequested: root.newChatRequested()
            onNewGroupChatRequested: root.newGroupChatRequested()
            onInfoRequested: root.infoRequested()
        }
    }

    Component {
        id: listViewComponent
        ChatListPage {
            anchors.fill: parent

            onNewChatRequested: root.newChatRequested()
            onNewGroupChatRequested: root.newGroupChatRequested()
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: window.useDesktopView ? (d.isLandscapeMode ? splitViewComponent : stackViewComponent) :
                                                 listViewComponent
    }

    Component.onCompleted: controllers.chats.groupInvitationRejected.connect(window.navigateBack)

    function navigateBack(transition) { return loader.item.navigateBack(transition) }

    function refresh() {}

    function processPickedAttachment(fileUrls, attachmentType) {
        const url = fileUrls[fileUrls.length - 1]
        if (attachmentType === AttachmentTypes.picture) {
            controllers.messages.sendPictureMessage(url, attachmentType)
        }
        else {
            controllers.messages.sendFileMessage(url, attachmentType)
        }
    }
}
