import QtQuick 2.6
import QtQuick.Controls 2.0

Item {
    id: unicodeIcon

    property alias unicodeText: unicodeTxt.text
    property alias fontSize: unicodeTxt.font.pointSize

    Text {
        id: unicodeTxt
        //    text: "\uf023"
        font.pointSize: Theme.ff(14)
        color: mainAppColor
        anchors.centerIn: parent
        //anchors.left: parent.left
        //anchors.verticalCenter: parent.verticalCenter
        //leftPadding: 10
    }
}


