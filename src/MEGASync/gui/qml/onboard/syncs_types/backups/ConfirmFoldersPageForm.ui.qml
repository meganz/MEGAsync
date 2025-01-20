import QtQuick.Layouts 1.15

import common 1.0

import components.pages 1.0

import backups 1.0

import onboard 1.0

import BackupCandidatesProxyModel 1.0
import BackupCandidates 1.0

FooterButtonsPage {
    id: root

    property alias enableConfirmHeader: confirmHeader.enabled

    readonly property int spacing: Constants.defaultComponentSpacing

    footerButtons.rightPrimary {
        text: BackupsStrings.backUp
        icons.source: Images.database
        enabled: backupCandidatesAccess.globalError === BackupCandidates.NONE
                    || backupCandidatesAccess.globalError === BackupCandidates.SDK_CREATION
    }

    ColumnLayout {
        id: externalLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: root.spacing

        HeaderTexts {
            id: confirmHeader

            title: OnboardingStrings.confirmBackupFoldersTitle
        }

        ConfirmFoldersContent {
            id: contentItem

            Layout.preferredWidth: parent.width
        }
    }

}
