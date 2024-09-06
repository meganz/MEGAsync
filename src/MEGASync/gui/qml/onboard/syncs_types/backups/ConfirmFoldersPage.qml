import QtQuick 2.15

import common 1.0

ConfirmFoldersPageForm {
    id: root

    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToFinal(bool success)

    footerButtons {
        leftPrimary.text: Strings.skip
        leftPrimary.onClicked: {
            window.close();
        }

        rightSecondary.onClicked: {
            root.confirmFoldersMoveToSelect();
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            enableConfirmHeader = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsComponentAccess.createBackups(window.syncOrigin);
        }
    }

    Connections {
        target: backupCandidatesAccess

        function onNoneSelected() {
            root.confirmFoldersMoveToSelect();
        }
    }

    Connections {
        target: backupsComponentAccess

        function onBackupsCreationFinished(success) {
            footerButtons.enabled = true;
            enableConfirmHeader = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.confirmFoldersMoveToFinal(success);
        }
    }

}
