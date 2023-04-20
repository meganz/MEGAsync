ResumePageForm {

    width: parent.width
    height: parent.height

    buttonGroup.onClicked: {
        switch(button.type) {
            case InstallationTypeButton.Type.Sync:
                syncsFlow.state = syncs;
                break;
            case InstallationTypeButton.Type.Backup:
                //selectBackupFoldersPage.backupTable.backupModel.clean();
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
        console.debug("TODO: Open in preferences button clicked");
    }

    doneButton.onClicked: {
        console.debug("TODO: Done button clicked");
    }
}
