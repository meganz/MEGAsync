import QtQml 2.12
import Onboarding 1.0

FullSyncPageForm {

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncs;
        }

        nextButton.onClicked: {
            Onboarding.addSync(localFolderChooser.getSyncData());
        }
    }

    Connections {
        target: Onboarding

        onSyncSetupSucces: {
            localFolderChooser.reset();
        }
    }

}
