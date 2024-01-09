import QtQuick 2.15

ResumePageForm {
    id: root

    property int tabToOpen: 0

    signal resumePageMoveToSyncs
    signal resumePageMoveToSelectiveSyncs
    signal resumePageMoveToBackup

    buttonGroup.onClicked: {
        switch(button.type) {
            case SyncsType.Types.SYNC:
                root.resumePageMoveToSyncs();
                break;
            case SyncsType.Types.SELECTIVE_SYNC:
                root.resumePageMoveToSelectiveSyncs();
                break;
            case SyncsType.Types.BACKUP:
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
            syncButton.forceActiveFocus();
        }
    }
}
