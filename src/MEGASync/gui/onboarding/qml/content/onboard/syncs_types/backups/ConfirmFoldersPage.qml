// System
import QtQuick 2.12

// C++
import BackupsController 1.0
import BackupsModel 1.0

ConfirmFoldersPageForm {

    footerButtons {

        rightSecondary.onClicked: {
            BackupsModel.clean(true);
            backupsProxyModel.selectedFilterEnabled = false;
            backupsFlow.state = backupsFlow.selectBackup;
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            confirmHeader.enabled = false;
            disableTableButtons = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsProxyModel.createBackups();
        }
    }

    Connections {
        target: BackupsModel

        onNoneSelected: {
            footerButtons.rightSecondary.clicked();
        }

        onExistConflictsChanged: {
            if(BackupsModel.mConflictsNotificationText !== "") {
                if(BackupsModel.mGlobalError === BackupsModel.BackupErrorCode.SDKCreation) {
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
        target: BackupsController

        onBackupsCreationFinished: (success) => {
            footerButtons.enabled = true;
            confirmHeader.enabled = true;
            disableTableButtons = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            if(success) {
                syncsPanel.state = syncsPanel.finalState;
            } else {
                stepPanel.state = stepPanel.step4Error;
            }
        }
    }

}
