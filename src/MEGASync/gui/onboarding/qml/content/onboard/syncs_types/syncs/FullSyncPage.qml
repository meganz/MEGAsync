// System
import QtQml 2.12

//C++
import Syncs 1.0

FullSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsFlow.state = syncType;
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            syncsCpp.addSync(localFolderChooser.getSyncData());
        }
    }

    Syncs {
        id: syncsCpp

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            syncsPanel.state = finalState;
            localFolderChooser.reset();
        }

        onCantSync: {
            root.enabled = true;
            footerButtons.nextButton.icons.busyIndicatorVisible = false;
            console.log(message);
            localFolderChooser.folderField.error = true;
            localFolderChooser.folderField.hint.text = message;
            localFolderChooser.folderField.hint.visible = true;
        }
    }
}
