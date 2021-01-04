import QtQuick 2.12
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import "../base"
import "../theme"

Control {
    id: chatMessage
    height: chatMessagePlaceholder.height

    property int thisIndex: -1
    property var thisDay

    property double maxWidth: 0
    readonly property real leftIndent: isMessageAlignedLeft ? avatarSize + contentSpacing + Theme.margin : avatarSize + contentSpacing

    property string body: ""
    property string displayTime: ""
    property string nickname
    property bool isOwnMessage: false
    property string statusIcon: ""

    property bool isBroken: false
    property string messageId: ""
    property bool inRow: false
    property bool firstInRow: true

    property string attachmentId: ""
    property bool attachmentTypeIsFile: false
    property bool attachmentTypeIsPicture: false
    property bool attachmentIsLoading: false
    property bool attachmentIsLoaded: false
    property int attachmentBytesTotal: 0
    property string attachmentDisplaySize: ""
    property string attachmentDisplayText: ""
    property string attachmentIconPath: ""
    property int attachmentPictureThumbnailWidth: 0
    property int attachmentPictureThumbnailHeight: 0
    property string attachmentDisplayProgress: ""
    property int attachmentBytesLoaded: 0
    property bool attachmentFileExists: false

    signal saveAttachmentAs(string messageId)
    signal openContextMenu(string messageId, var mouse, var contextMenu)

    readonly property int stdCheckHeight: 14
    readonly property int stdTopMargin: 15
    readonly property int stdSmallMargin: 5
    readonly property int stdRadiusHeight: 20
    readonly property int stdRectangleSize: 22
    property int leftBottomRadiusHeight: inRow ? 4 : stdRadiusHeight

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
        readonly property bool hasAttachment: attachmentId.length > 0
        readonly property color background: isOwnMessage ? "#59717D" : Theme.mainBackgroundColor
        readonly property bool isPicture: attachmentTypeIsPicture
        readonly property double defaultRadius: 4
    }

    ChatMessagePlaceholder {
        id: chatMessagePlaceholder
    }

//  Smart items
    UnreadMessagesSeparator {
        id: unreadMessagesSeparator
    }
}
