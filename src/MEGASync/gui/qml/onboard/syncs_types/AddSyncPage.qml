import QtQuick 2.15

import syncs 1.0
import SyncsComponents 1.0
import SyncInfo 1.0

AddSyncPageForm {
    id: root

    signal syncSetupSucceed(bool isFullSync)
    signal moveBack

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    Component.onCompleted: {
        syncsComponentAccess.enteredOnSync();
        syncsComponentAccess.clearLocalFolderErrorHint();
        syncsComponentAccess.clearRemoteFolderErrorHint();
    }

    localFolderChooser.folderField {
        hint {
            text: syncsDataAccess.localError
            visible: syncsDataAccess.localError.length !== 0
        }
        error: syncsDataAccess.localError.length !== 0
    }

    remoteFolderChooser.folderField {
        hint {
            text: syncsDataAccess.remoteError
            visible: syncsDataAccess.remoteError.length !== 0
        }
        error: syncsDataAccess.remoteError.length !== 0
    }

    localFolderChooser.onButtonClicked: {
        syncsComponentAccess.chooseLocalFolderButtonClicked(localFolderChooser.chosenPath);
    }

    remoteFolderChooser.onButtonClicked: {
        syncsComponentAccess.chooseRemoteFolderButtonClicked();
    }

    footerButtons {
        rightSecondary.onClicked: {
            root.moveBack();
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsComponentAccess.syncButtonClicked(localFolderChooser.chosenPath, remoteFolderChooser.chosenPath);
        }
    }

    Connections {
        id: syncsComponentAccessConnection

        target: syncsComponentAccess
        enabled: root.enabled

        function onLocalFolderChosen(localFolderPath) {
            localFolderChooser.chosenPath = localFolderPath;
        }

        function onRemoteFolderChosen(remoteFolderPath) {
            remoteFolderChooser.chosenPath = remoteFolderPath;
        }
    }

    Connections {
        target: syncsDataAccess

        function onSyncSetupSuccess(isFullSync) {
            enableScreen();

            root.syncSetupSucceed(isFullSync);
        }

        function onSyncSetupFailed() {
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
