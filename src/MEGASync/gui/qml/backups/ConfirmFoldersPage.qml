import QtQuick 2.15

import common 1.0

ConfirmFoldersPageForm {
    id: root

    signal openExclusionsDialog
    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToFinal(bool success)
    signal createBackups(int origin)

    footerButtons {
        leftSecondary.onClicked: {
            root.openExclusionsDialog();
        }

        rightSecondary.onClicked: {
            root.confirmFoldersMoveToSelect();
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            root.createBackups(window.syncOrigin);
        }
    }

    Connections {
        target: backupCandidatesAccess

        function onNoneSelected() {
            root.confirmFoldersMoveToSelect();
        }
    }

    Connections {
        target: backupCandidatesComponentAccess

        function onBackupsCreationFinished(success) {
            footerButtons.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.confirmFoldersMoveToFinal(success);
        }
    }

}
