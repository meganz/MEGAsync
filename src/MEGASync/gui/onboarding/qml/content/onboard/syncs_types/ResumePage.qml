// System
import QtQuick 2.12

// C++
import Onboarding 1.0

ResumePageForm {

    property bool comesFromSync: true

    buttonGroup.onClicked: {
        switch(button.type) {
            case SyncsType.Sync:
                syncsPanel.state = syncsFlow;
                break;
            case SyncsType.Backup:
                syncsPanel.state = backupsFlow;
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    footerButtons {

        rightSecondary.onClicked: {
            Onboarding.openPreferences(typeSelected === SyncsType.Types.SelectiveSync
                                       || typeSelected === SyncsType.Types.FullSync);
        }

        rightPrimary.onClicked: {
            Onboarding.exitLoggedIn();
        }
    }

    Component.onCompleted: {
        switch(typeSelected) {
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
                console.warn("ResumePage: typeSelected does not exist -> " + typeSelected);
                break;
        }
    }
}
