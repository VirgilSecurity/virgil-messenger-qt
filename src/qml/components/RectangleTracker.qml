import QtQuick 2.15

Rectangle {
    property var sourceItem: undefined

    color: "transparent"
    border.color: "red"
    z: 1000

    visible: sourceItem
    x: sourceItem ? sourceItem.x : 0
    y: sourceItem ? sourceItem.y : 0
    width: sourceItem ? sourceItem.width : 0
    height: sourceItem ? sourceItem.height : 0
    parent: sourceItem ? sourceItem.parent : window.contentItem
}
