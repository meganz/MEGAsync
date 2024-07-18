import QtQuick 2.15

import Syncs 1.0
import ChooseLocalFolder 1.0

SelectiveSyncPageForm {
    id: root

    signal selectiveSyncMoveToBack
    signal selectiveSyncMoveToSuccess

    /*
    localFolderChooser.folderField.hint.text : syncs.localError
    localFolderChooser.folderField.hint.visible : (syncs.localError.length !== 0)
    localFolderChooser.folderField.error : (syncs.localError.length !== 0)
    */

    footerButtons {
        leftSecondary.onClicked: {
            syncsComponentAccess.openExclusionsDialog(localFolderChooser.choosenPath);
        }

        rightSecondary.onClicked: {
            root.selectiveSyncMoveToBack();
        }

        rightPrimary.onClicked: {
            /*
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;

            remoteFolderChooser.folderField.hint.visible = false;
            remoteFolderChooser.folderField.error = false;

            var localFolderError = false;
            if (localFolderChooser.choosenPath.length === 0) {
                localFolderError = true;
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = SyncsStrings.invalidLocalPath;
                localFolderChooser.folderField.hint.visible = true;
            }

            var remoteFolderError = false;
            if (remoteFolderChooser.choosenPath.length === 0) {
                remoteFolderError = true;
                remoteFolderChooser.folderField.error = true;
                remoteFolderChooser.folderField.hint.text = SyncsStrings.invalidRemotePath;
                remoteFolderChooser.folderField.hint.visible = true;
            }

            if (localFolderError || remoteFolderError) {
                return;
            }

            if (localFolderChooser.choosenPath === localFolder.getDefaultFolder(syncs.defaultMegaFolder)
                    && !localFolder.createFolder(localFolderChooser.choosenPath)) {
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = SyncsStrings.canNotSyncPermissionError;
                localFolderChooser.folderField.hint.visible = true;
                return;
            }
            */

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
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            remoteFolderChooser.reset();
            root.selectiveSyncMoveToSuccess();
        }

        /*
        onCantSync: (message, localFolderError) => {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;

            if(message.length === 0) {
                return;
            }

            var folderChooser;
            if(localFolderError) {
                folderChooser = localFolderChooser;
            }
            else {
                folderChooser = remoteFolderChooser;
            }
            folderChooser.folderField.error = true;
            folderChooser.folderField.hint.text = message;
            folderChooser.folderField.hint.visible = true;

            console.log("Selective sync can't sync, message -> " + message);
        }
        */

        onLocalErrorChanged: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;

            var errorMessage = syncs.localError;
            var isErrorMessageEmpty = (errorMessage.length !== 0);

            localFolderChooser.folderField.error = isErrorMessageEmpty;
            localFolderChooser.folderField.hint.text = errorMessage;
            localFolderChooser.folderField.hint.visible = isErrorMessageEmpty;

            if (isErrorMessageEmpty) {
                console.log("Selective sync can't sync, local error message -> " + errorMessage);
            }
        }

        onRemoteErrorChanged: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;

            var errorMessage = syncs.remoteError;
            var isErrorMessageEmpty = (errorMessage.length !== 0);

            remoteFolderChooser.folderField.error = isErrorMessageEmpty;
            remoteFolderChooser.folderField.hint.text = errorMessage;
            remoteFolderChooser.folderField.hint.visible = isErrorMessageEmpty;

            if (isErrorMessageEmpty) {
                console.log("Selective sync can't sync, remote error message -> " + errorMessage);
            }
        }

        /*
        onLocalErrorChanged: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
        }

        onRemoteErrorChanged: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
        }
        */
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            localFolderChooser.forceActiveFocus();
        }

        /*
        function onLanguageChanged() {
            if (localFolderChooser.folderField.hint.visible || remoteFolderChooser.folderField.hint.visible) {
                footerButtons.rightPrimary.clicked();
            }
        }
        */
    }
}
