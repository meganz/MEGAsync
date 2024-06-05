import QtQuick 2.15

import common 1.0

import BackupsModel 1.0

ConfirmFoldersPageForm {
    id: root

    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToFinal(bool success)

    footerButtons {
        leftSecondary.onClicked: {
            var folderPaths = backupsProxyModelRef.getSelectedFolders();
            backupsAccess.openExclusionsDialog(folderPaths);
        }

        rightSecondary.onClicked: {
            backupsModelAccess.clean(true);
            root.confirmFoldersMoveToSelect();
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
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
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.confirmFoldersMoveToFinal(success);
        }
    }

}
