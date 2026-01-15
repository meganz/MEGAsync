import QtQuick 2.15

import components.steps 1.0

import BackupCandidates 1.0

BackupsFlow {
    id: root

    required property var backupsContentItemRef

    selectFoldersPage: Component {
        id: selectFoldersPageComponent

        SelectFoldersPage {
            id: selectFoldersPageItem
        }
    }

    confirmFoldersPage: Component {
        id: confirmFoldersPageComponent

        ConfirmFoldersPage {
            id: confirmFoldersPageItem
        }
    }

    onBackupFlowMoveToFinal: (success) => {
        if (success) {
            backupsContentItemRef.state = backupsContentItemRef.resume;
        }
    }
}
