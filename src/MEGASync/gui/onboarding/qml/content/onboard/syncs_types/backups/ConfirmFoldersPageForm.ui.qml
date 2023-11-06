// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.TextFields 1.0 as MegaTextFields

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

// C++
import BackupsModel 1.0
import QmlDeviceName 1.0

SyncsPage {
    id: root
    property alias enableConfirmHeader: confirmHeader.enabled

    footerButtons.rightPrimary {
        text: OnboardingStrings.backUp
        icons.source: Images.database
        enabled: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.NONE
                    || (backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
                            && backupsModelAccess.existsOnlyGlobalError)
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        Header {
            id: confirmHeader

            title: OnboardingStrings.confirmBackupFoldersTitle
        }

        ColumnLayout {
            id: mainLayout

            Layout.preferredWidth: parent.width
            spacing: 24

            ConfirmTable {
                id: confirmFoldersTable
            }

            MegaTextFields.TextField {
                id: deviceField

                colors.text: Styles.textFaded
                Layout.preferredWidth: parent.width
                Layout.leftMargin: -deviceField.sizes.focusBorderWidth
                title: OnboardingStrings.backupTo
                leftIconSource: Images.database
                textField.readOnly: true
                textField.text: "/" + deviceName.name
            }
        }
    }

    QmlDeviceName {
        id: deviceName
    }
}
