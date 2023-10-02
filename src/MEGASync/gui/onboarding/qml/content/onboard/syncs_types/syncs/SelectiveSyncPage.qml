// System
import QtQml 2.12

//C++
import Syncs 1.0
import ChooseLocalFolder 1.0

SelectiveSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsFlow.state = syncType;
        }

        rightPrimary.onClicked: {
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;

            remoteFolderChooser.folderField.hint.visible = false;
            remoteFolderChooser.folderField.error = false;

            var localFolderError = false
            if (localFolderChooser.choosenPath.length === 0) {
                localFolderError = true
                localFolderChooser.folderField.error = true
                localFolderChooser.folderField.hint.text = qsTr("Invalid directory.")
                localFolderChooser.folderField.hint.visible = true
            }

            var remoteFolderError = false
            if (remoteFolderChooser.choosenPath.length === 0) {
                remoteFolderError = true
                remoteFolderChooser.folderField.error = true
                remoteFolderChooser.folderField.hint.text = qsTr("Invalid directory.")
                remoteFolderChooser.folderField.hint.visible = true
            }

            if (localFolderError || remoteFolderError) {
                return
            }

            if (!localFolder.createFolder(localFolderChooser.choosenPath)) {
                localFolderChooser.folderField.error = true
                localFolderChooser.folderField.hint.text = qsTr("Couldn't create directory : " + localFolderChooser.choosenPath)
                localFolderChooser.folderField.hint.visible = true

                return;
            }

            root.enabled = false
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true
            syncsCpp.addSync(localFolderChooser.choosenPath, remoteFolderChooser.choosenPath)
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
            syncsPanel.state = finalState;
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
    }

}
