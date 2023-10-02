// System
import QtQml 2.12

//C++
import Syncs 1.0
import ChooseLocalFolder 1.0

FullSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsFlow.state = syncType;
        }

        rightPrimary.onClicked: {
            localFolderChooser.folderField.hint.visible = false
            localFolderChooser.folderField.error = false

            if (localFolderChooser.choosenPath.length === 0) {
                localFolderChooser.folderField.error = true
                localFolderChooser.folderField.hint.text = qsTr("Invalid directory.")
                localFolderChooser.folderField.hint.visible = true
            }
            else if (localFolder.createFolder(localFolderChooser.choosenPath)) {
                root.enabled = false;
                footerButtons.rightPrimary.icons.busyIndicatorVisible = true;

                syncsCpp.addSync(localFolderChooser.choosenPath)
            }
            else {
                localFolderChooser.folderField.error = true
                localFolderChooser.folderField.hint.text = qsTr("Couldn't create directory : " + localFolderChooser.choosenPath)
                localFolderChooser.folderField.hint.visible = true
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
