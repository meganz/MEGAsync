// C++
import Onboarding 1.0

ResumePageForm {

    property bool comesFromSync: true

    width: parent.width
    height: parent.height

    buttonGroup.onClicked: {
        switch(button.type) {
            case InstallationTypeButton.Type.Sync:
                syncsFlow.state = syncs;
                break;
            case InstallationTypeButton.Type.Backup:
                selectBackupFoldersPage.backupTable.backupModel.clean();
                syncsFlow.state = selectBackup;
                break;
            case InstallationTypeButton.Type.Fuse:
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
