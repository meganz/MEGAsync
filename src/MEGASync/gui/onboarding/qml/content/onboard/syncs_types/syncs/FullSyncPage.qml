// System
import QtQml 2.12

// C++
import Onboarding 1.0

FullSyncPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsFlow.state = syncs;
        }

        rightPrimary.onClicked: {
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            Onboarding.addSync(localFolderChooser.getSyncData());
        }
    }

    Connections {
        target: Onboarding

        onSyncSetupSuccess: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            syncsFlow.state = finalState;
            localFolderChooser.reset();
        }
    }

}
