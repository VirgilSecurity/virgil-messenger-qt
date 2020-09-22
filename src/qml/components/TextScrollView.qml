import QtQuick 2.15
import QtQuick.Controls 2.15

ScrollView {
    clip: true

    Component.onCompleted: contentItem.boundsBehavior = Flickable.StopAtBounds
}
