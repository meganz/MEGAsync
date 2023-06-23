// C++
import Onboarding 1.0

ResumePageForm {

    property bool comesFromSync: true

    buttonGroup.onClicked: {
        switch(button.type) {
            case SyncsType.Sync:
                mainFlow.state = syncs;
                break;
            case SyncsType.Backup:
                mainFlow.state = backupsFlow;
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    preferencesButton.onClicked: {
        Onboarding.openPreferences(lastTypeSelected === SyncsType.Types.Sync);
    }

    doneButton.onClicked: {
        onboardingWindow.close();
    }
}
