import QtQuick 2.15
import QtQuick.Controls 2.15

NavigationStackView {
    id: root

    Component {
        id: accountSettingsMainComponent
        AccountSettingsMainPage {
            onEditProfileRequested: root.navigatePush(editProfileComponent)
            onBackupKeyRequested: root.navigatePush(backupKeyComponent)
        }
    }

    Component {
        id: backupKeyComponent
        BackupKeyPage {
            onKeyBackuped: window.navigateBack()
        }
    }

    Component {
        id: editProfileComponent
        EditProfilePage {
            onVerificationRequested: root.navigatePush(verifyProfileComponent, { "codeType": codeType })
        }
    }

    Component {
        id: verifyProfileComponent
        VerifyProfilePage {
            onVerified: window.navigateBack()
        }
    }

    Component.onCompleted: navigatePush(accountSettingsMainComponent)
}
