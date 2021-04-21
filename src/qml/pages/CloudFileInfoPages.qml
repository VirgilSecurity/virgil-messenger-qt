import QtQuick 2.15
import QtQuick.Controls 2.15

NavigationStackView {
    id: root

    Component {
        id: cloudFileInfoMainComponent
        CloudFileInfoMainPage {
            onAddMembersRequested: root.navigatePush(addCloudFolderMembersComponent)
        }
    }

    Component {
        id: addCloudFolderMembersComponent
        CloudFolderMembersPage {
            buttonText: qsTr("Add members")

            onSelected: {
                appState.addSharedFolderMembers(contacts)
                window.navigateBack()
            }
        }
    }

    Component.onCompleted: navigateReplace(cloudFileInfoMainComponent)
}
