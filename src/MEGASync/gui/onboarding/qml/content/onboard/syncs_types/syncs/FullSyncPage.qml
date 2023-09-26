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
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;

            if (localFolder.createFolder(localFolderChooser.localChoosenPath)) {
                syncsCpp.addSync(localFolderChooser.localChoosenPath)
            }
            else {
                localFolderChooser.folderField.error = true
                localFolderChooser.folderField.hint.text = qsTr("Couldn't create directory : " + localFolderChooser.localChoosenPath)
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

        onCancelSync: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
        }
    }
}
