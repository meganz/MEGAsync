// C++
import BackupsModel 1.0

SelectFoldersPageForm {

    footerButtons {

        rightSecondary.onClicked: {
            if(syncsPanel.comesFromResumePage) {
                syncsPanel.typeSelected = syncsPanel.previousTypeSelected;
                syncsPanel.state = syncsPanel.finalState;
            } else {
                syncsPanel.state = syncsPanel.syncType;
            }
        }

        rightPrimary.onClicked: {
            BackupsModel.check();
            backupsProxyModel.selectedFilterEnabled = true;
            backupsFlow.state = backupsFlow.confirmBackup;
            if(BackupsModel.mConflictsNotificationText !== "") {
                stepPanel.state = stepPanel.step4Warning;
            } else {
                stepPanel.state = stepPanel.step4;
            }
        }
    }

}
