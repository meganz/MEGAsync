// Local
import Onboard.Syncs_types 1.0

SyncTypePageForm {

    footerButtons {

        rightSecondary.onClicked: {
            syncsFlow.state = syncType;
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.syncType) {
                case SyncsType.FullSync:
                    syncsFlow.state = fullSync;
                    break;
                case SyncsType.SelectiveSync:
                    syncsFlow.state = selectiveSync;
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


