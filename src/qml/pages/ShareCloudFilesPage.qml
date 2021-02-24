import QtQuick 2.15

MultiSelectContactsPage {
    appState: app.stateManager.shareCloudFilesState
    headerTitle: qsTr("Share to")
//    onFinished: {
//        controllers.cloudFiles.shareFiles()
//        app.stateManager.goBack()
//    }
}
