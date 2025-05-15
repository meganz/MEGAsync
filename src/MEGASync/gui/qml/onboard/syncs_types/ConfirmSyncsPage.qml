import QtQuick 2.15

import SyncsComponents 1.0
import SyncInfo 1.0

ConfirmSyncsPageForm {
    id: root

    signal selectiveSyncMoveToSuccess
    signal fullSyncMoveToSuccess

    footerButtons {
        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsComponentAccess.syncButtonClicked();
        }

        rightSecondary.visible: false;
        leftPrimary.visible: false;
    }

    footerButtons {
        leftSecondary.onClicked: {
            syncsComponentAccess.exclusionsButtonClicked();
        }

        rightSecondary.onClicked: {
            root.moveBack();
        }

        rightPrimary.onClicked: {
            root.moveNext();
        }
    }

    Connections {
        target: syncsDataAccess

        function onSyncSetupSuccess(isFullSync) {
            enableScreen();

            if (isFullSync) {
                root.fullSyncMoveToSuccess();
            }
            else {
                root.selectiveSyncMoveToSuccess();
            }
        }
    }
}
