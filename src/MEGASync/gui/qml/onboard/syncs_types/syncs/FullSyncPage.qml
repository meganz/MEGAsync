import QtQuick 2.15

import onboard 1.0

import Syncs 1.0
import ChooseLocalFolder 1.0

FullSyncPageForm {
    id: root

    signal fullSyncMoveToBack
    signal fullSyncMoveToSuccess

    footerButtons {

        rightSecondary.onClicked: {
            root.fullSyncMoveToBack()
        }

        rightPrimary.onClicked: {
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;

            if (localFolderChooser.choosenPath.length === 0) {
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = OnboardingStrings.invalidLocalPath;
                localFolderChooser.folderField.hint.visible = true;
            }
            else if (localFolderChooser.choosenPath !== localFolder.getDefaultFolder(syncs.defaultMegaFolder)
                     || localFolder.createFolder(localFolderChooser.choosenPath)) {
                root.enabled = false;
                footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
                syncs.addSync(localFolderChooser.choosenPath);
            }
            else {
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = OnboardingStrings.canNotSyncPermissionError;
                localFolderChooser.folderField.hint.visible = true;
            }
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
            root.fullSyncMoveToSuccess();
            localFolderChooser.reset();
        }

        onCantSync: (message, localFolderError) => {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;

            if(message.length === 0) {
                return;
            }

            localFolderChooser.folderField.error = true;
            localFolderChooser.folderField.hint.text = message;
            localFolderChooser.folderField.hint.visible = true;

            console.log("Full sync can't sync, message -> " + message);
        }
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            localFolderChooser.forceActiveFocus();
        }
    }
}
