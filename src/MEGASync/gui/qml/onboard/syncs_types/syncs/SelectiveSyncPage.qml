import QtQuick 2.15

import onboard 1.0

import Syncs 1.0
import ChooseLocalFolder 1.0

SelectiveSyncPageForm {
    id: root

    signal selectiveSyncMoveToBack
    signal selectiveSyncMoveToSuccess

    footerButtons {

        rightSecondary.onClicked: {
            root.selectiveSyncMoveToBack()
        }

        rightPrimary.onClicked: {
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;

            remoteFolderChooser.folderField.hint.visible = false;
            remoteFolderChooser.folderField.error = false;

            var localFolderError = false;
            if (localFolderChooser.choosenPath.length === 0) {
                localFolderError = true;
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = OnboardingStrings.invalidLocalPath;
                localFolderChooser.folderField.hint.visible = true;
            }

            var remoteFolderError = false
            if (remoteFolderChooser.choosenPath.length === 0) {
                remoteFolderError = true;
                remoteFolderChooser.folderField.error = true;
                remoteFolderChooser.folderField.hint.text = OnboardingStrings.invalidRemotePath;
                remoteFolderChooser.folderField.hint.visible = true;
            }

            if (localFolderError || remoteFolderError) {
                return;
            }

            if (localFolderChooser.choosenPath === localFolder.getDefaultFolder(syncs.defaultMegaFolder)
                    && !localFolder.createFolder(localFolderChooser.choosenPath)) {
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = OnboardingStrings.canNotSyncPermissionError;
                localFolderChooser.folderField.hint.visible = true;
                return;
            }

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
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            localFolderChooser.forceActiveFocus();
        }
    }
}
