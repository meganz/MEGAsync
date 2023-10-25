// C++
import BackupsModel 1.0

SelectFoldersPageForm {
    id: root

    signal selectFolderMoveToBack
    signal selectFolderMoveToConfirm

    footerButtons {

        rightSecondary.onClicked: {
            root.selectFolderMoveToBack()
        }

        rightPrimary.onClicked: {
            backupsModelAccess.check();
            backupsProxyModel.selectedFilterEnabled = true;
            if(backupsModelAccess.conflictsNotificationText !== "") {
                stepPanel.state = stepPanel.step4Warning;
            } else {
                stepPanel.state = stepPanel.step4;
            }

            root.selectFolderMoveToConfirm()
        }
    }

}
