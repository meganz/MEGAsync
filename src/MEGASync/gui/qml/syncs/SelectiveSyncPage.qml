import QtQuick 2.15

import Syncs 1.0
import ChooseLocalFolder 1.0

SelectiveSyncPageForm {
    id: root

    signal selectiveSyncMoveToBack
    signal selectiveSyncMoveToSuccess

    localFolderChooser.folderField.hint.text : syncs.localError
    localFolderChooser.folderField.hint.visible : syncs.localError.length !== 0
    localFolderChooser.folderField.error : syncs.localError.length !== 0

    remoteFolderChooser.folderField.hint.text : syncs.remoteError
    remoteFolderChooser.folderField.hint.visible : syncs.remoteError.length !== 0
    remoteFolderChooser.folderField.error : syncs.remoteError.length !== 0

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
            syncs.addSync(localFolderChooser.choosenPath, remoteFolderChooser.choosenPath);
        }
    }

    ChooseLocalFolder {
        id: localFolder
    }

    Syncs {
        id: syncs

        onSyncSetupSuccess: {
            enableScreen();

            remoteFolderChooser.reset();
            root.selectiveSyncMoveToSuccess();
        }

        onLocalErrorChanged: {
            enableScreen();
        }

        onRemoteErrorChanged: {
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
