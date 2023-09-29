// System
import QtQuick 2.12

// C++
import Onboarding 1.0

ResumePageForm {

    buttonGroup.onClicked: {
        syncsPanel.previousTypeSelected = syncsPanel.typeSelected;
        switch(button.type) {
            case SyncsType.Sync:
                syncsPanel.state = syncsPanel.syncsFlow;
                break;
            case SyncsType.SelectiveSync:
                syncsPanel.state = syncsPanel.syncsFlow;
                syncsPanel.typeSelected = SyncsType.Types.SelectiveSync;
                break;
            case SyncsType.Backup:
                syncsPanel.state = syncsPanel.backupsFlow;
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    footerButtons {

        rightSecondary.onClicked: {
            Onboarding.openPreferences(syncsPanel.typeSelected === SyncsType.Types.SelectiveSync
                                       || syncsPanel.typeSelected === SyncsType.Types.FullSync);
        }

        rightPrimary.onClicked: {
            onboardingWindow.close();
        }
    }

    Component.onCompleted: {
        syncsPanel.comesFromResumePage = true;
        switch(syncsPanel.typeSelected) {
            case SyncsType.Types.SelectiveSync:
                state = stateSelectiveSync;
                break;
            case SyncsType.Types.FullSync:
                state = stateFullSync;
                break;
            case SyncsType.Types.Backup:
                state = stateBackup;
                break;
            default:
                console.warn("ResumePage: typeSelected does not exist -> "
                             + syncsPanel.typeSelected);
                break;
        }
    }
}
