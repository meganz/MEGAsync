import QtQuick 2.15

import syncs 1.0
import ChooseLocalFolder 1.0
import SyncInfo 1.0

SelectiveSyncPageForm {
    id: root

    signal selectiveSyncMoveToBack
    signal selectiveSyncMoveToSuccess
    signal fullSyncMoveToSuccess

    localFolderChooser.folderField.hint.text: syncsData.localError
    localFolderChooser.folderField.hint.visible: syncsData.localError.length !== 0
    localFolderChooser.folderField.error: syncsData.localError.length !== 0

    remoteFolderChooser.folderField.hint.text: syncsData.remoteError
    remoteFolderChooser.folderField.hint.visible: syncsData.remoteError.length !== 0
    remoteFolderChooser.folderField.error: syncsData.remoteError.length !== 0

    function enableScreen() {
        root.enabled = true;
        footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
    }

    footerButtons {
        leftSecondary.onClicked: {
            syncsComponentAccess.openExclusionsDialog(localFolderChooser.choosenPath);
        }

        rightSecondary.onClicked: {
            root.selectiveSyncMoveToBack();
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncs.addSync(isOnboarding ?  SyncInfo.ONBOARDING_ORIGIN : syncsComponentAccess.getSyncOrigin(),
                               localFolderChooser.choosenPath,
                               remoteFolderChooser.choosenPath);
        }
    }

    Connections {
        target: syncs

        function onSyncSetupSuccess() {
            var remotePath = remoteFolderChooser.choosenPath;

            enableScreen();
            remoteFolderChooser.reset();
            localFolderChooser.reset();

            if (remotePath === '/')
            {
                root.fullSyncMoveToSuccess();
            }
            else
            {
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
