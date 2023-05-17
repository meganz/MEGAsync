// System
import QtQml 2.12

// C++
import Onboarding 1.0

SelectiveSyncPageForm {
    id: root

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncs;
        }

        nextButton.onClicked: {
            root.enabled = false;
            footerButtons.nextButton.icons.busyIndicatorVisible = true;
            Onboarding.addSync(localFolderChooser.getSyncData(), remoteFolderChooser.getSyncData());
        }
    }

    Connections {
        target: Onboarding

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.nextButton.icons.busyIndicatorVisible = false;
            syncsFlow.state = finalState;
            localFolderChooser.reset();
            remoteFolderChooser.reset();
        }
    }

}
