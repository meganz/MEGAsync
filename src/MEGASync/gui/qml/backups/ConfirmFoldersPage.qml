import QtQuick 2.15

import common 1.0

import BackupsModel 1.0
import BackupsProxyModel 1.0

ConfirmFoldersPageForm {
    id: root

    required property BackupsProxyModel backupsProxyModelRef

    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToFinal(bool success)

    footerButtons {
        leftIcon.onClicked: {
            Qt.openUrlExternally(Links.desktopSyncApp);
        }

        rightSecondary.onClicked: {
            backupsModelAccess.clean(true);
            root.confirmFoldersMoveToSelect();
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            enableConfirmHeader = false;
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
            enableConfirmHeader = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.confirmFoldersMoveToFinal(success);
        }
    }

}
