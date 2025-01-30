import QtQuick 2.15

import Syncs 1.0
import ChooseLocalFolder 1.0
import SyncInfo 1.0

SelectiveSyncPageForm {
    id: root

    signal selectiveSyncMoveToBack
    signal selectiveSyncMoveToSuccess
    signal fullSyncMoveToSuccess;

    localFolderChooser.folderField.hint.text: root.syncs.localError
    localFolderChooser.folderField.hint.visible: root.syncs.localError.length !== 0
    localFolderChooser.folderField.error: root.syncs.localError.length !== 0

    remoteFolderChooser.folderField.hint.text: root.syncs.remoteError
    remoteFolderChooser.folderField.hint.visible: root.syncs.remoteError.length !== 0
    remoteFolderChooser.folderField.error: root.syncs.remoteError.length !== 0

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
            root.syncs.addSync(isOnboarding ?  SyncInfo.ONBOARDING_ORIGIN : syncsComponentAccess.origin,
                               localFolderChooser.choosenPath,
                               remoteFolderChooser.choosenPath);
        }
    }

    Connections {
        target: root.syncs

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
