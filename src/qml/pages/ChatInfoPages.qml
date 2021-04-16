import QtQuick 2.15
import QtQuick.Controls 2.15

NavigationStackView {
    id: root

    Component {
        id: chatInfoMainComponent
        ChatInfoMainPage {
            onEditRequested: root.navigatePush(editChatInfoComponent)
            onAddMembersRequested: root.navigatePush(groupChatMembersComponent)
        }
    }

    Component {
        id: editChatInfoComponent
        EditChatInfoPage {
            onSaved: window.navigateBack()
        }
    }

    Component {
        id: groupChatMembersComponent
        GroupChatMembersPage {
            loadingText: qsTr("Adding group members...")
            buttonText: qsTr("Add members")

            onSelected: appState.addGroupMembers(contacts)

            Component.onCompleted: controllers.chats.groupMembersAdded.connect(onGroupMembersAdded)

            function onGroupMembersAdded() { window.navigateBack() }
        }
    }

    Component.onCompleted: navigateReplace(chatInfoMainComponent)
}
