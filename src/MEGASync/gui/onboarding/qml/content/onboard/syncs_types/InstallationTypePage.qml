InstallationTypePageForm {

    footerButtons {

        previousButton.onClicked: {
            mainFlow.state = computerName;
        }

        nextButton.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Sync:
                    mainFlow.state = syncs;
                    break;
                case SyncsType.Backup:
                    mainFlow.state = backupsFlow;
                    break;
                default:
                    console.error("Button type does not exist -> "
                                  + buttonGroup.checkedButton.type);
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
