// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

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

    footerButtons.rightPrimary {
        text: OnboardingStrings.backUp
        icons.source: Images.database
        enabled: BackupsModel.mGlobalError === BackupsModel.BackupErrorCode.None
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        Header {
            title: OnboardingStrings.confirmBackupFoldersTitle
        }

        ColumnLayout {
            id: mainLayout

            Layout.preferredWidth: parent.width
            spacing: 24

            ConfirmTable {}

            MegaTextFields.TextField {
                id: folderField

                Layout.preferredWidth: parent.width + 2 * folderField.sizes.focusBorderWidth
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
