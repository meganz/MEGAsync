InstallationTypePageForm {
    id: root

    signal installationTypeMoveToBack;
    signal installationTypeMoveToSync;
    signal installationTypeMoveToBackup;

    footerButtons {

        rightSecondary.onClicked: {
            root.installationTypeMoveToBack()
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.Sync:
                    root.installationTypeMoveToSync()
                    break;
                case SyncsType.Types.Backup:
                    root.installationTypeMoveToBackup()
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
