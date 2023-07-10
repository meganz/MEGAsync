InstallationTypePageForm {

    footerButtons {

        rightSecondary.onClicked: {
            syncsPanel.state = computerName;
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.Sync:
                    syncsPanel.state = syncs;
                    break;
                case SyncsType.Types.Backup:
                    syncsPanel.state = backupsFlow;
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
            footerButtons.rightPrimary.enabled = true;
        }
    }

}
