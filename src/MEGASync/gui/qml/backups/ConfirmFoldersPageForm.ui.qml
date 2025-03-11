import common 1.0

import components.pages 1.0

import BackupCandidatesProxyModel 1.0
import BackupCandidates 1.0

FooterButtonsPage {
    id: root

    readonly property int spacing: 24

    footerButtons {
        leftPrimary.visible: false
        leftSecondary {
            text: Strings.setExclusions
            visible: true
            enabled: backupCandidatesAccess.globalError === BackupCandidates.NONE
                        || backupCandidatesAccess.globalError === BackupCandidates.SDK_CREATION
        }
        rightTertiary.visible: true
        rightPrimary {
            text: BackupsStrings.backUp
            icons.source: Images.database
            enabled: backupCandidatesAccess.globalError === BackupCandidates.NONE
                        || backupCandidatesAccess.globalError === BackupCandidates.SDK_CREATION
        }
    }

    ConfirmFoldersContent {
        id: contentItem

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

}
