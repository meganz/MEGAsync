// C++
import BackupsModel 1.0

SelectFoldersPageForm {

    footerButtons {

        rightSecondary.onClicked: {
            syncsPanel.state = syncType;
        }

        rightPrimary.onClicked: {
            BackupsModel.check();
            backupsProxyModel.selectedFilterEnabled = true;
            backupsFlow.state = confirmBackup;
            if(BackupsModel.mConflictsNotificationText !== "") {
                stepPanel.state = stepPanel.step4Warning;
            } else {
                stepPanel.state = stepPanel.step4;
            }
        }
    }

}
