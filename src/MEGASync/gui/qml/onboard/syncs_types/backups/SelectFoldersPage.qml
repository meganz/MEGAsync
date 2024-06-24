import BackupsModel 1.0

SelectFoldersPageForm {
    id: root

    signal selectFolderMoveToBack
    signal selectFolderMoveToConfirm

    footerButtons {
        rightSecondary.onClicked: {
            root.selectFolderMoveToBack();
        }

        rightPrimary.onClicked: {
            root.selectFolderMoveToConfirm();
            backupsModelAccess.check();
        }
    }

}
