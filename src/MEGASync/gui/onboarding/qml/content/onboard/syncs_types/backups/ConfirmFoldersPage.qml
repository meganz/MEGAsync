// System
import QtQuick 2.15

// C++
import BackupsController 1.0
import BackupsModel 1.0

ConfirmFoldersPageForm {
    id: root

    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToSuccess

    footerButtons {

        rightSecondary.onClicked: {
            backupsModelAccess.clean(true);
            backupsProxyModel.selectedFilterEnabled = false;
            root.confirmFoldersMoveToSelect()
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            enableConfirmHeader = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsProxyModel.createBackups();
        }
    }

    Connections {
        target: backupsModelAccess

        function onNoneSelected() {
            root.confirmFoldersMoveToSelect()
        }

        function onExistConflictsChanged() {
            if(backupsModelAccess.globalError !== backupsModelAccess.BackupErrorCode.NONE) {
                if(backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION) {
                    stepPanel.state = stepPanel.step4Error;
                } else {
                    stepPanel.state = stepPanel.step4Warning;
                }
            } else {
                stepPanel.state = stepPanel.step4;
            }
        }
    }

    Connections {
        target: backupsProxyModel

        function onBackupsCreationFinished(success) {
            footerButtons.enabled = true;
            enableConfirmHeader = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            if(success) {
                root.confirmFoldersMoveToSuccess()
            } else {
                stepPanel.state = stepPanel.step4Error;
            }
        }
    }
}
