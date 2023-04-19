import QtQml 2.12
import Onboarding 1.0

SelectiveSyncPageForm {

    function getSyncData() {
        var localFolder = localFolderChooser.getSyncData();
        var handle = remoteFolderChooser.getSyncData();
        return { localFolder, handle };
    }

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
        }
    }

}
