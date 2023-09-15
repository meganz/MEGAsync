// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0
import LoginController 1.0

SyncsPage {

    property alias buttonGroup: buttonGroup

    footerButtons.rightPrimary.enabled: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        Header {
            title: LoginControllerAccess.state === LoginController.EMAIL_CONFIRMED ? OnboardingStrings.welcomeToMEGA : OnboardingStrings.letsGetYouSetUp
            description: OnboardingStrings.chooseInstallation
            spacing: 36
            descriptionWeight: Font.DemiBold
        }

        ButtonGroup {
            id: buttonGroup
        }

        ColumnLayout {
            spacing: 12

            SyncsHorizontalButton {
                id: syncButton

                Layout.leftMargin: -syncButton.focusBorderWidth
                Layout.rightMargin: -syncButton.focusBorderWidth
                title: OnboardingStrings.sync
                description: OnboardingStrings.syncButtonDescription
                imageSource: Images.sync
                type: SyncsType.Types.Sync
                ButtonGroup.group: buttonGroup
            }

            SyncsHorizontalButton {
                id: backupsButton

                Layout.leftMargin: -backupsButton.focusBorderWidth
                Layout.rightMargin: -backupsButton.focusBorderWidth
                title: OnboardingStrings.backup
                description: OnboardingStrings.backupButtonDescription
                imageSource: Images.installationTypeBackups
                type: SyncsType.Backup
                ButtonGroup.group: buttonGroup
            }
        }
    }

}
