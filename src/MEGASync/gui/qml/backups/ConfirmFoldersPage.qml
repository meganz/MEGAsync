import QtQuick 2.15

import BackupsModel 1.0

ConfirmFoldersPageForm {
    id: root

    required property var backupsProxyModelRef

    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToFinal(bool success)

    footerButtons {

        rightSecondary.onClicked: {
            backupsModelAccess.clean(true);
            root.confirmFoldersMoveToSelect();
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            enableConfirmHeader = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsProxyModelRef.createBackups();
        }
    }

    Connections {
        target: backupsModelAccess

        function onNoneSelected() {
            root.confirmFoldersMoveToSelect();
        }
    }

    Connections {
        target: backupsProxyModelRef

        function onBackupsCreationFinished(success) {
            footerButtons.enabled = true;
            enableConfirmHeader = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.confirmFoldersMoveToFinal(success);
        }
    }

    /*
    Connections {
        target: onboardingWindow

        function onLanguageChanged() {
            if (footerButtons.rightPrimary.enabled && backupsModelAccess.globalError > backupsModelAccess.BackupErrorCode.NONE) {
                footerButtons.rightPrimary.clicked();
            }
        }
    }
    */
}
