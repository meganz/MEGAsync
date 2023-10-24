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
            backupsModel.clean(true);
            backupsProxyModel.selectedFilterEnabled = false;
            root.confirmFoldersMoveToSelect()
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            confirmHeader.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsProxyModel.createBackups();
        }
    }

    Connections {
        target: backupsModel

        function onNoneSelected() {
            root.confirmFoldersMoveToSelect()
        }

        function onExistConflictsChanged() {
            if(backupsModel.conflictsNotificationText !== "") {
                if(backupsModel.globalError === backupsModel.BackupErrorCode.SDKCreation) {
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
        target: backupsController

        function onBackupsCreationFinished(success) {
            footerButtons.enabled = true;
            confirmHeader.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            if(success) {
                root.confirmFoldersMoveToSuccess()
            } else {
                stepPanel.state = stepPanel.step4Error;
            }
        }
    }
}
