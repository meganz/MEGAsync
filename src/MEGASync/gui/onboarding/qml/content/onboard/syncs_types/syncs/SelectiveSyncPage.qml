// System
import QtQml 2.12

//C++
import Syncs 1.0

SelectiveSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            if(syncsPanel.comesFromResumePage) {
                syncsPanel.typeSelected = syncsPanel.previousTypeSelected;
                syncsPanel.state = syncsPanel.finalState;
            } else {
                syncsPanel.state = syncsPanel.syncType;
            }
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsCpp.addSync(localFolderChooser.localTest, remoteFolderChooser.remoteTest);
        }
    }

    Syncs {
        id: syncsCpp

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            syncsPanel.selectiveSyncDone = true;
            syncsPanel.state = finalState;
            localFolderChooser.reset();
            remoteFolderChooser.reset();
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
            } else {
                folderChooser = remoteFolderChooser;
            }
            folderChooser.folderField.error = true;
            folderChooser.folderField.hint.text = message;
            folderChooser.folderField.hint.visible = true;

            console.log("Selective sync can't sync, message -> " + message);
        }

        onCancelSync: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
        }
    }

}
