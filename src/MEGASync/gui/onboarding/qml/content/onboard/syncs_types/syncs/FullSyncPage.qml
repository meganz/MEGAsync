// System
import QtQml 2.12

//C++
import Syncs 1.0
import ChooseLocalFolder 1.0

//Local
import Onboard 1.0

FullSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            if(syncsPanel.navInfo.comesFromResumePage && syncsPanel.navInfo.syncDone) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                syncsPanel.state = syncsPanel.finalState;
            } else {
                syncsFlow.state = syncsFlow.syncType;
            }
        }

        rightPrimary.onClicked: {
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;

            if (localFolderChooser.choosenPath.length === 0) {
                localFolderChooser.folderField.error = true;
                localFolderChooser.folderField.hint.text = OnboardingStrings.invalidLocalPath;
                localFolderChooser.folderField.hint.visible = true;
            }
            else if (localFolderChooser.choosenPath !== localFolder.getDefaultFolder(syncsCpp.DEFAULT_MEGA_FOLDER) || localFolder.createFolder(localFolderChooser.choosenPath)) {
                root.enabled = false;
                footerButtons.rightPrimary.icons.busyIndicatorVisible = true;

                syncsCpp.addSync(localFolderChooser.choosenPath);
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
        id: syncsCpp

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            syncsPanel.navInfo.fullSyncDone = true;
            syncsPanel.state = finalState;
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
}
