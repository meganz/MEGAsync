// Local
import Onboard.Syncs_types 1.0

SyncTypePageForm {

    footerButtons {

        rightSecondary.onClicked: {
            syncsPanel.state = syncType;
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.FullSync:
                    syncsPanel.state = fullSync;
                    break;
                case SyncsType.Types.SelectiveSync:
                    syncsPanel.state = selectiveSync;
                    break;
                default:
                    console.error("Button type does not exist -> "
                                  + buttonGroup.checkedButton.syncType);
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


