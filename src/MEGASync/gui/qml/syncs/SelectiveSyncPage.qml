import QtQuick 2.15

import syncs 1.0
import SyncsComponents 1.0
import SyncInfo 1.0

SelectiveSyncPageForm {
    id: root

    signal selectiveSyncMoveToBack
    signal selectiveSyncMoveToSuccess
    signal fullSyncMoveToSuccess

    localFolderChooser.folderField {
        hint {
            text: syncsDataAccess.localError
            visible: syncsDataAccess.localError.length !== 0
        }
        error: syncsDataAccess.localError.length !== 0
        text: syncsDataAccess.localFolderCandidate
    }

    remoteFolderChooser.folderField {
        hint {
            text: syncsDataAccess.remoteError
            visible: syncsDataAccess.remoteError.length !== 0
        }
        error: syncsDataAccess.remoteError.length !== 0
        text: syncsDataAccess.remoteFolderCandidate
    }

    localFolderChooser.onButtonClicked: {
        syncsComponentAccess.chooseLocalFolderButtonClicked();
    }

    remoteFolderChooser.onButtonClicked: {
        syncsComponentAccess.chooseRemoteFolderButtonClicked();
    }

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    footerButtons {
        leftSecondary.onClicked: {
            syncsComponentAccess.exclusionsButtonClicked();
        }

        rightSecondary.onClicked: {
            root.selectiveSyncMoveToBack();
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsComponentAccess.syncButtonClicked(localFolderChooser.chosenPath, remoteFolderChooser.chosenPath);
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

        function onLocalErrorChanged() {
            enableScreen();
        }

        function onRemoteErrorChanged() {
            enableScreen();
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            localFolderChooser.forceActiveFocus();
        }
    }
}
