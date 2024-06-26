import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts

import onboard 1.0

import LoginController 1.0

SyncsPage {
    id: root

    property alias buttonGroup: buttonGroupComp
    property alias syncButton: syncButtonItem

    footerButtons.rightPrimary.enabled: false

    ColumnLayout {
        id: mainLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        Header {
            id: headerItem

            title: loginControllerAccess.newAccount
                   ? OnboardingStrings.welcomeToMEGA
                   : OnboardingStrings.letsGetYouSetUp
            description: OnboardingStrings.chooseInstallation
            spacing: 36
            descriptionWeight: Font.DemiBold
            descriptionColor: colorStyle.textPrimary
        }

        ButtonGroup {
            id: buttonGroupComp
        }

        ColumnLayout {
            id: buttonsLayout

            spacing: 12

            SyncsHorizontalButton {
                id: syncButtonItem

                Layout.leftMargin: -syncButtonItem.focusBorderWidth
                Layout.rightMargin: -syncButtonItem.focusBorderWidth
                title: OnboardingStrings.sync
                description: OnboardingStrings.syncButtonDescription
                imageSource: Images.sync
                type: SyncsType.Types.SYNC
                ButtonGroup.group: buttonGroupComp
            }

            SyncsHorizontalButton {
                id: backupsButton

                Layout.leftMargin: -backupsButton.focusBorderWidth
                Layout.rightMargin: -backupsButton.focusBorderWidth
                title: OnboardingStrings.backup
                description: OnboardingStrings.backupButtonDescription
                imageSource: Images.installationTypeBackups
                type: SyncsType.Types.BACKUP
                ButtonGroup.group: buttonGroupComp
            }
        }
    }

}
