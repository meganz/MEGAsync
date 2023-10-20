// System
import QtQuick 2.12

// C++
import Onboarding 1.0

ResumePageForm {
    id: root

    signal resumePageMoveToSyncs
    signal resumePageMoveToSelectiveSyncs
    signal resumePageMoveToBackup

    buttonGroup.onClicked: {
        switch(button.type) {
            case SyncsType.Sync:
                root.resumePageMoveToSyncs()
                break;
            case SyncsType.SelectiveSync:
                root.resumePageMoveToSelectiveSyncs()
                break;
            case SyncsType.Backup:
                root.resumePageMoveToBackup()
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    footerButtons {

        rightSecondary.onClicked: {
            Onboarding.openPreferences(syncsPanel.navInfo.typeSelected === SyncsType.Types.SelectiveSync
                                       || syncsPanel.navInfo.typeSelected === SyncsType.Types.FullSync);
        }

        rightPrimary.onClicked: {
            onboardingWindow.close();
        }
    }
}
