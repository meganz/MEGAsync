import common 1.0

import BackupsModel 1.0

SelectFoldersPageForm {
    id: root

    signal selectFolderMoveToConfirm

    footerButtons {
        rightSecondary.onClicked: {
            window.close();
        }

        rightPrimary.onClicked: {
            root.selectFolderMoveToConfirm();
            backupsModelAccess.check();
        }
    }

}
