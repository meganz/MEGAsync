import QtQuick 2.15

import SyncsComponents 1.0
import SyncInfo 1.0

ConfirmSyncsPageForm {
    id: root

    signal syncSetupFailed
    signal syncSetupSucceed(bool isFullSync)
    signal moveBack

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    footerButtons {
        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsComponentAccess.confirmSyncCandidateButtonClicked();
        }

        rightSecondary.visible: false;
        leftPrimary.visible: false;
    }

    confirmTable.onMoveBack: {
        root.moveBack()
    }

    Connections {
        target: syncsDataAccess

        function onSyncCandidatesSetupSuccess(isFullSync) {
            enableScreen();

            root.syncSetupSucceed(isFullSync);
        }

        function onSyncCandidatesSetupFailed() {
            enableScreen();

            root.syncSetupFailed();
        }
    }
}
