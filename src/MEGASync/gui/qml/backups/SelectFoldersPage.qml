import common 1.0

import BackupsModel 1.0

SelectFoldersPageForm {
    id: root

    signal selectFolderMoveToBack
    signal selectFolderMoveToConfirm

    footerButtons {
        leftIcon.onClicked: {
            Qt.openUrlExternally(Links.desktopSyncApp);
        }

        rightSecondary.onClicked: {
            if(root.isOnboardingRef) {
                root.selectFolderMoveToBack();
            }
            else {
                window.close();
            }
        }

        rightPrimary.onClicked: {
            root.selectFolderMoveToConfirm();
            backupsModelAccess.check();
        }
    }

}
