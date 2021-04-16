import QtQuick 2.15
import QtQuick.Controls 2.15

NavigationStackView {
    id: root

    Component {
        id: mainComponent
        MainPage {
            onAccountSettingsRequested: root.navigatePush(accountSettingsComponent)
//            onNewChatRequested: root.navigatePush(newChatComponent)
//            onNewGroupChatRequested: root.navigatePush(newGroupChatComponent)
//            onChatInfoRequested: root.navigatePush(chatInfoComponent)
            onCloudFileSharingRequested: root.navigatePush(cloudFileInfoComponent)
            onCloudFileSetMembersRequested: root.navigatePush(setCloudFolderMembersComponent, { "name": name })
        }
    }

    Component {
        id: accountSelectionComponent
        AccountSelectionPages {
            onChatListRequested: root.openChatList()
        }
    }

    Component {
        id: accountSettingsComponent
        AccountSettingsPages {}
    }

    // DIALOGS / WIZARDS

    Component {
        id: newChatComponent
        NewChatPage {
            onCreated: window.navigateBack()
        }
    }

    Component {
        id: newGroupChatComponent
        NewGroupChatPages {
            onCreated: window.navigateBack()
        }
    }

    Component {
        id: chatInfoComponent
        ChatInfoPages {}
    }

    Component {
        id: cloudFileInfoComponent
        CloudFileInfoPages {}
    }

    Component {
        id: setCloudFolderMembersComponent
        CloudFolderMembersPage {
            buttonText: qsTr("Create folder")

            property string name: ""

            onSelected: {
                appState.createSharedFolder(name, contacts)
                window.navigateBack()
            }
        }
    }

    Component.onCompleted: {
        var appState = app.stateManager.startState
        appState.chatListRequested.connect(root.openChatList)
        appState.accountSelectionRequested.connect(root.openAccountSelection)

        controllers.users.signedIn.connect(root.openChatList)
        controllers.users.signedOut.connect(root.openAccountSelection)
        controllers.users.signInErrorOccured.connect(root.openAccountSelection)
    }

    function openChatList() { navigateReplace(mainComponent) }

    function openAccountSelection() { navigateReplace(accountSelectionComponent) }
}
