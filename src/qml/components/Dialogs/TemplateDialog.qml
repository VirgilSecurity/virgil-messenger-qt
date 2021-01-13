import QtQuick 2.12
import QtQuick.Controls 2.12

import "../"
import "../../theme"

Popup {
    id: templateDialog

    property alias backgroundRect: backgroundRectangle

    // Bahavior settings
    property real exitTransitionDuration: 200

    // Content settings
    property string title: "Dialog"
    property string text: "Text"

    // Button settings
    property string acceptedButtonText: qsTr('Ok')
    property string rejectedButtonText: qsTr('Cancel')

    // Signals
    signal accepted()
    signal rejected()

    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    padding: Theme.margin

    QtObject {
        id: d
        readonly property real radius: 10
        readonly property real popupImplicitWidth: Math.min(420, root.width * 0.8)
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.InOutQuad;
                duration: Theme.animationDuration; easing.overshoot: 1}
            NumberAnimation {property: "scale"; from: 0.8; to: 1.0; easing.type: Easing.InOutQuad;
                duration: Theme.animationDuration; easing.overshoot: 2}
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {property: "opacity"; from: 1.0; to: 0.0; easing.type: Easing.InOutQuad;
                duration: exitTransitionDuration; easing.overshoot: 1}
            NumberAnimation {property: "scale"; from: 1.0; to: 0.8; easing.type: Easing.InOutQuad;
                duration: exitTransitionDuration; easing.overshoot: 1}
        }
    }

    Rectangle {
        id: backgroundRectangle
        focus: false
        color: Theme.mainBackgroundColor
        radius: d.radius
        anchors.centerIn: parent
        implicitWidth: d.popupImplicitWidth
        height: contentColumn.height + 2 * Theme.margin

        Column {
            id: contentColumn
            anchors {
                left: parent.left
                leftMargin: Theme.margin
                right: parent.right
                rightMargin: Theme.margin
                top: parent.top
                topMargin: Theme.margin
            }

            spacing: Theme.spacing

            Text {
                width: parent.width
                text: templateDialog.title
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(17)
                wrapMode: Text.WordWrap
            }

            Text {
                width: parent.width
                text: templateDialog.text
                color: Theme.primaryTextColor
                font.pointSize: UiHelper.fixFontSz(15)
                wrapMode: Text.WordWrap
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.spacing

                FormPrimaryButton {
                    text: acceptedButtonText
                    onClicked: {
                        accepted()
                        templateDialog.close()
                    }
                }

                FormSecondaryButton {
                    text: rejectedButtonText
                    onClicked: {
                        rejected()
                        templateDialog.close()
                    }
                }
            }
        }
    }

    background: Item {}
}
