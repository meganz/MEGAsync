import QtQuick 2.15

import common 1.0

ResumePageForm {
    id: root

    property int tabToOpen: 0

    signal resumePageMoveToSyncs
    signal resumePageMoveToSelectiveSyncs
    signal resumePageMoveToBackup

    buttonGroup.onClicked: {
        switch(button.type) {
            case Constants.SyncType.SYNC:
                root.resumePageMoveToSyncs();
                break;
            case Constants.SyncType.SELECTIVE_SYNC:
                root.resumePageMoveToSelectiveSyncs();
                break;
            case Constants.SyncType.BACKUP:
                root.resumePageMoveToBackup();
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    footerButtons {
        rightSecondary.onClicked: {
            onboardingAccess.openPreferences(tabToOpen);
        }

        rightPrimary.onClicked: {
            window.close();
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            syncButton.forceActiveFocus();
        }
    }
}
