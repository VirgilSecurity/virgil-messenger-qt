import QtQuick 2.15
import QtQuick.Controls 2.15

NavigationStackView {
    id: root

    Component {
        id: mainComponent
        MainPage {
            onAccountSettingsRequested: root.navigatePush(accountSettingsComponent)
            onNewChatRequested: root.navigatePush(newChatComponent)
            onNewGroupChatRequested: root.navigatePush(newGroupChatComponent)
            onChatInfoRequested: root.navigatePush(chatInfoComponent)
            onCloudFileSharingRequested: root.navigatePush(cloudFileInfoComponent)
            onCloudFileSetMembersRequested: root.navigatePush(setCloudFolderMembersComponent, { "name": name })
        }
    }

    Component {
        id: accountSelectionComponent
        AccountSelectionPages {
            onChatListRequested: d.openChatList()
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
            onCreated: d.openChatList()
        }
    }

    Component {
        id: newGroupChatComponent
        NewGroupChatPages {
            onCreated: d.openChatList()
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

    QtObject {
        id: d
        function openChatList() { navigateReplace(mainComponent) }
        function openAccountSelection() { navigateReplace(accountSelectionComponent) }
    }

    Component.onCompleted: {
        var appState = app.stateManager.startState
        appState.chatListRequested.connect(d.openChatList)
        appState.accountSelectionRequested.connect(d.openAccountSelection)

        messenger.signedIn.connect(d.openChatList)
        messenger.signedUp.connect(d.openChatList)
        messenger.keyDownloaded.connect(d.openChatList)
        messenger.signedOut.connect(d.openAccountSelection)
        messenger.signInErrorOccured.connect(d.openAccountSelection)
    }
}
