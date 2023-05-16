// C++
import Onboarding 1.0

ResumePageForm {

    property bool comesFromSync: true

    width: parent.width
    height: parent.height

    buttonGroup.onClicked: {
        switch(button.type) {
            case SyncsType.Sync:
                syncsFlow.state = syncs;
                break;
            case SyncsType.Backup:
                syncsFlow.state = selectBackup;
                break;
            case SyncsType.Fuse:
                break;
            default:
                console.error("Button type does not exist -> " + button.type);
                break;
        }
    }

    preferencesButton.onClicked: {
        Onboarding.openPreferences(comesFromSync);
    }

    doneButton.onClicked: {
        Onboarding.exitLoggedIn();
    }
}
