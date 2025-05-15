import QtQuick 2.15

import SyncsComponents 1.0
import SyncInfo 1.0

ConfirmSyncsPageForm {
    id: root

    signal syncSetupFailed
    signal syncSetupSucceed(bool isFullSync)

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    footerButtons {
        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsComponentAccess.syncButtonClicked();
        }

        rightSecondary.visible: false;
        leftPrimary.visible: false;
    }

    Connections {
        target: syncsDataAccess

        function onSyncSetupSuccess(isFullSync) {
            enableScreen();

            root.syncSetupSucceed(isFullSync);
        }

        function onSyncSetupFailed() {
            enableScreen();

            root.syncSetupFailed();
        }
    }
}
