import QtQuick 2.15

NavigationStackView {
    id: root

    property string name: ""

    signal created()

    Component {
        id: groupChatNameComponent
        GroupChatNamePage {
            onNamed: root.navigatePush(groupChatMembersComponent, { "name": name })
        }
    }

    Component {
        id: groupChatMembersComponent
        GroupChatMembersPage {
            loadingText: qsTr("Creating new group...")
            buttonText: qsTr("Create group")

            property string name: ""

            onSelected: appState.createGroupChat(name, contacts)
        }
    }

    Component.onCompleted: {
        navigateReplace(groupChatNameComponent)

        controllers.chats.chatCreated.connect(created)
    }
}
