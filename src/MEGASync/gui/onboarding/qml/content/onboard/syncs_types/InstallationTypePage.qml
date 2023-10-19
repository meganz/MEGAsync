InstallationTypePageForm {
    id: root

    signal moveToBack;
    signal moveToSync;
    signal moveToBackup;

    footerButtons {

        rightSecondary.onClicked: {
            root.moveToBack()
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.Sync:
                    root.moveToSync()
                    break;
                case SyncsType.Types.Backup:
                    root.moveToBackup()
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
