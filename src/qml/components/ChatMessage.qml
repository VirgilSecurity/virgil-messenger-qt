import QtQuick 2.12
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "../base"
import "../theme"

Control {
    id: chatMessage
    height: chatMessageBody.height

    property int thisIndex: -1
    property var thisDay

    property double maxWidth: 0
    readonly property real leftIndent: isMessageAlignedLeft ? avatarSize + contentSpacing + Theme.margin : avatarSize + contentSpacing

    property bool isOwnMessage: false
    property string statusIcon: ""
    property int attachmentPictureThumbnailWidth: 0
    property int attachmentPictureThumbnailHeight: 0
    property string messageId: ""

    signal saveAttachmentAs(string messageId)
    signal openContextMenu(string messageId, var mouse, var contextMenu)

    readonly property int stdCheckHeight: 14
    readonly property int stdTopMargin: 15
    readonly property int stdSmallMargin: 5
    readonly property int stdRadiusHeight: 20
    readonly property int stdRectangleSize: 22
    property int leftBottomRadiusHeight: model.inRow ? 4 : stdRadiusHeight

    readonly property int avatarSize: 30
    readonly property int contentSpacing: 12

    readonly property bool isMessageAlignedLeft: {
        if (!Platform.isMobile) {
            if (isOwnMessage) {
                if (chatMessage.width > 500) {
                    return true
                } else {
                    return false
                }
            } else {
                return true
            }
        }

        if (Platform.isMobile) {
            if (isOwnMessage) {
                return false
            } else {
                return true
            }
        }
    }


    Behavior on leftBottomRadiusHeight {
        NumberAnimation { duration: Theme.animationDuration}
    }

    QtObject {
        id: d
        readonly property bool hasAttachment: model.attachmentId.length > 0
        readonly property color background: isOwnMessage ? "#59717D" : Theme.mainBackgroundColor
        readonly property bool isPicture: attachmentTypeIsPicture
        readonly property double defaultRadius: 4
    }

    ChatMessageBody {
        id: chatMessageBody

        ParallelAnimation {
            id: chatMessageBodyAnimation
            NumberAnimation { target: chatMessageBody; property: 'y'; from: chatMessageBody.height; to: 0; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
            NumberAnimation { target: chatMessageBody; property: 'scale'; from: 0; to: 1; duration: Theme.animationDuration; easing.type: Easing.InOutCubic }
            NumberAnimation { target: chatMessageBody; property: 'opacity'; from: 0; to: 1; duration: Theme.animationDuration }
        }
    }

//  Smart items
//    UnreadMessagesSeparator {
//        id: unreadMessagesSeparator
//    }

    Component.onCompleted: {
        if (index === 0) {
            console.log("YYYYYYYYYYYYYYY messageAddAnimationEnabled", messageAddAnimationEnabled)
        }

        if (index === 0 && messageAddAnimationEnabled) {
            chatMessageBodyAnimation.restart()
            messageAddAnimationEnabled = false
        }
    }
}
