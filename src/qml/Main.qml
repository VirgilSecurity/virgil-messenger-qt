import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQuick.LocalStorage 2.0

import "login/login.js" as Backend
import "helpers/ui"

ApplicationWindow {
    id: rootWindow
    visible: true
    title: qsTr("Virgil IoTKit Qt Demo")
    minimumWidth: 550
    minimumHeight: 500

    //
    //  Properties
    //
    property color backGroundColor : "#394454"
    property color mainAppColor: "#6fda9c"
    property color mainTextCOlor: "#f0f0f0"
    property var dataBase

    //
    //  Connections
    //
    Connections {
        target: Messenger

        onFireError: {
            showPopupError(errorText)
            stackView.push("qrc:/qml/login/Login.qml")
        }

        onFireConnecting: {
        }

        onFireReady: {
            stackView.push("qrc:/qml/chat/ContactPage.qml")
        }
    }


    //
    //  UI
    //

    // Main stackview
    StackView {
        id: stackView
        focus: true
        anchors.fill: parent
    }

    // After loading show initial Login Page
    Component.onCompleted: {
        stackView.push("qrc:/qml/login/Login.qml")
        dataBase = userDataBase()
    }

    // Popup to show messages or warnings on the bottom postion of the screen
    Popup {
        id: inform
    }

    // Show Popup message
    function showPopupError(message) {
        inform.popupColor = "#b44"
        inform.popupColorText = "#ffffff"
        inform.popupView.popMessage = message
        inform.popupView.open()
    }

    function showPopupInform(message) {
        inform.popupColor = "#FFFACD"
        inform.popupColorText = "#00"
        inform.popupView.popMessage = message
        inform.popupView.open()
    }

    function showPopupSucces(message) {
        inform.popupColor = "#66CDAA"
        inform.popupColorText = "#00"
        inform.popupView.popMessage = message
        inform.popupView.open()
    }

    // Create and initialize the database
    function userDataBase() {
        var db = LocalStorage.openDatabaseSync("UserLoginApp", "1.0", "Login example!", 1000000);
        db.transaction(function(tx) {
            tx.executeSql('CREATE TABLE IF NOT EXISTS UserDetails(username TEXT, password TEXT, hint TEXT)');
        })

        return db;
    }

}
