import QtQuick 2.15

NavigationStackView {
    id: root

    signal chatListRequested()

    Component {
        id: accountSelectionOptionsComponent
        AccountSelectionOptionsPage {
            onSignIn: {
                controllers.users.signIn(username)
                root.chatListRequested()
            }
            onSignInUsername: root.navigatePush(restoreAccountUsernameComponent)
            onSignUp: root.navigatePush(signUpComponent)
        }
    }

    Component {
        id: restoreAccountUsernameComponent
        RestoreAccountUsernamePage {
            onUsernameValidated: root.navigatePush(restoreAccountOptionsComponent, { "username": username })
        }
    }

    Component {
        id: restoreAccountOptionsComponent
        RestoreAccountOptionsPage {
            onDownloadKeyRequested: root.navigatePush(downloadKeyComponent, { "username": username })
        }
    }

    Component {
        id: downloadKeyComponent
        DownloadKeyPage {}
    }

    Component {
        id: signUpComponent
        SignUpPage {}
    }

    Component.onCompleted: navigateReplace(accountSelectionOptionsComponent)
}
