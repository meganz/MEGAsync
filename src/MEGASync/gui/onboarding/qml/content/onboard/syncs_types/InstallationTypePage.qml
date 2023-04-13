InstallationTypePageForm {

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = computerName;
        }

        nextButton.enabled: false
        nextButton.onClicked: {
            switch(installationTypePage.buttonGroup.checkedButton.type) {
                case InstallationTypeButton.Type.Sync:
                    syncsFlow.state = syncs;
                    break;
                case InstallationTypeButton.Type.Backup:
                    syncsFlow.state = selectBackup;
                    break;
                case InstallationTypeButton.Type.Fuse:
                default:
                    console.error("Button type does not exist -> "
                                  + installationTypePage.buttonGroup.checkedButton.type);
                    break;
            }
        }
    }

    buttonGroup.onCheckStateChanged: {
        if(buttonGroup.checkedButton != null) {
            footerButtons.nextButton.enabled = true;
        }
    }

}
