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
            onCreated: {
                if (window.useDesktopView) {
                    d.closeNewChatDialog()
                }
            }
        }
    }

    Component {
        id: newGroupChatComponent
        NewGroupChatPages {
            onCreated: {
                if (window.useDesktopView) {
                    d.closeNewChatDialog()
                }
            }
        }
    }

    Component {
        id: chatPageComponent
        ChatPage {
            onInfoRequested: root.navigatePush(chatInfoComponent)
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
        readonly property string chatId: controllers.chats.current.id
        readonly property bool isChatOpened: chatId.length > 0

        onIsChatOpenedChanged: {
            if (window.useDesktopView) {
                return
            }

            if (isChatOpened) {
                if (root.depth === 2) {
                    closeNewChatDialog()
                }
                root.navigatePush(chatPageComponent)
            } else {
                root.navigatePop()
            }
        }

        function openChatList() { navigateReplace(mainComponent) }

        function openAccountSelection() { navigateReplace(accountSelectionComponent) }

        function closeNewChatDialog() { root.navigatePop(StackView.ReplaceTransition) }
    }

    Component.onCompleted: {
        controllers.users.userLoaded.connect(d.openChatList)
        controllers.users.userNotLoaded.connect(d.openAccountSelection)
        controllers.users.initialSignIn()
    }
}
