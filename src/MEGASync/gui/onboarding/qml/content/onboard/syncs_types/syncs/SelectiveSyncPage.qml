import QtQml 2.12
import Onboarding 1.0

SelectiveSyncPageForm {

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncs;
        }

        nextButton.onClicked: {
            Onboarding.addSync(localFolderChooser.getSyncData(), remoteFolderChooser.getSyncData());
        }
    }

    Connections {
        target: Onboarding

        onSyncSetupSucces: {
            syncsFlow.state = finalState;
            localFolderChooser.reset();
            remoteFolderChooser.reset();
        }
    }

}
