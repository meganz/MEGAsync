// System
import QtQml 2.12

// C++
import Onboarding 1.0

SelectiveSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsPanel.state = syncs;
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            localFolderChooser.folderField.hint.visible = false;
            localFolderChooser.folderField.error = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            Onboarding.addSync(localFolderChooser.getSyncData(), remoteFolderChooser.getSyncData());
        }
    }

    Connections {
        target: Onboarding

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            syncsPanel.state = finalState;
            localFolderChooser.reset();
            remoteFolderChooser.reset();
        }

        onCantSync: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            console.log(message);
            localFolderChooser.folderField.error = true;
            localFolderChooser.folderField.hint.text = message;
            localFolderChooser.folderField.hint.visible = true;
        }
    }

}
