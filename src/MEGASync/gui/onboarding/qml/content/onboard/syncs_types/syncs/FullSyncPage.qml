// System
import QtQml 2.12

// C++
import Onboarding 1.0

FullSyncPageForm {
    id: root

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncs;
        }

        nextButton.onClicked: {
            root.enabled = false;
            footerButtons.nextButton.busyIndicatorVisible = true;
            Onboarding.addSync(localFolderChooser.getSyncData());
        }
    }

    Connections {
        target: Onboarding

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.nextButton.busyIndicatorVisible = false;
            localFolderChooser.reset();
        }
    }

}
