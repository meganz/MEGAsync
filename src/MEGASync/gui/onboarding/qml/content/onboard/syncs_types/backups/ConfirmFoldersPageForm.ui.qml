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

    property alias folderField: folderField
    property alias confirmHeader: confirmHeader

    footerButtons.rightPrimary {
        text: OnboardingStrings.backUp
        icons.source: Images.database
        enabled: BackupsModel.mGlobalError === BackupsModel.BackupErrorCode.None
                    || (BackupsModel.mGlobalError === BackupsModel.BackupErrorCode.SDKCreation
                            && BackupsModel.mExistsOnlyGlobalError)
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
                id: folderField

                Layout.preferredWidth: parent.width
                Layout.leftMargin: -folderField.sizes.focusBorderWidth
                title: OnboardingStrings.backupTo
                leftIcon.source: Images.database
                textField.readOnly: true
                textField.text: "/" + deviceName.name
            }
        }
    }

    QmlDeviceName {
        id: deviceName
    }
}
