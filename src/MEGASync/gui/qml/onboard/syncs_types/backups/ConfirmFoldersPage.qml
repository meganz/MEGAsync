import QtQuick 2.15

import common 1.0
import BackupsComponent 1.0

ConfirmFoldersPageForm {
    id: root

    required property BackupsComponent backupsComponentAccessRef

    signal confirmFoldersMoveToSelect
    signal confirmFoldersMoveToFinal(bool success)
    signal createBackups(int syncOrigin)

    footerButtons {
        leftPrimary.text: Strings.skip
        leftPrimary.onClicked: {
            window.close();
        }

        rightSecondary.onClicked: {
            root.confirmFoldersMoveToSelect();
        }

        rightPrimary.onClicked: {
            footerButtons.enabled = false;
            enableConfirmHeader = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            root.createBackups(window.syncOrigin);
        }
    }

    Connections {
        target: backupsComponentAccessRef.data

        function onNoneSelected() {
            root.confirmFoldersMoveToSelect();
        }
    }

    Connections {
        target: backupsComponentAccessRef

        function onBackupsCreationFinished(success) {
            footerButtons.enabled = true;
            enableConfirmHeader = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            root.confirmFoldersMoveToFinal(success);
        }
    }

}
