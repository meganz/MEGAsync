import QtQuick 2.15

import Onboarding 1.0

ResumePageForm {
    id: root

    property int tabToOpen: 0

    signal resumePageMoveToSyncs
    signal resumePageMoveToSelectiveSyncs
    signal resumePageMoveToBackup

    buttonGroup.onClicked: {
        switch(button.type) {
            case SyncsType.Sync:
                root.resumePageMoveToSyncs();
                break;
            case SyncsType.SelectiveSync:
                root.resumePageMoveToSelectiveSyncs();
                break;
            case SyncsType.Backup:
                root.resumePageMoveToBackup();
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    footerButtons {

        rightSecondary.onClicked: {
            onboardingAccess.openPreferences(tabToOpen)
        }

        rightPrimary.onClicked: {
            onboardingWindow.close();
        }
    }

    Connections {
        target: onboardingWindow

        function onInitializePageFocus() {
            syncButton.checked = true;
            syncButton.forceActiveFocus();
        }
    }
}
