// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

SyncsPage {

    property alias buttonGroup: buttonGroup

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 32
        }
        spacing: 24

        Header {
            title: OnboardingStrings.installationTypeTitle
            description: OnboardingStrings.installationTypeDescription
        }

        ButtonGroup {
            id: buttonGroup
        }

        ColumnLayout {
            spacing: 20

            InstallationTypeButton {
                id: syncButton

                title: OnboardingStrings.sync
                description: OnboardingStrings.syncButtonDescription
                imageSource: Images.sync
                type: InstallationTypeButton.Type.Sync
                ButtonGroup.group: buttonGroup
            }

            InstallationTypeButton {
                id: backupsButton

                title: OnboardingStrings.backup
                description: OnboardingStrings.backupButtonDescription
                imageSource: Images.installationTypeBackups
                type: InstallationTypeButton.Type.Backup
                ButtonGroup.group: buttonGroup
            }

            InstallationTypeButton {
                id: fuseButton

                title: OnboardingStrings.fuse
                description: OnboardingStrings.fuseButtonDescription
                imageSource: Images.fuse
                type: InstallationTypeButton.Type.Fuse
                ButtonGroup.group: buttonGroup
            }
        }
    }

}
