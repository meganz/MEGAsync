// Local
import Onboard.Syncs_types 1.0

SyncTypePageForm {

    footerButtons {

        rightSecondary.onClicked: {
            if(syncsPanel.navInfo.comesFromResumePage) {
                syncsPanel.navInfo.typeSelected = syncsPanel.navInfo.previousTypeSelected;
                syncsPanel.state = syncsPanel.finalState;
            } else {
                syncsPanel.state = syncsPanel.syncType;
            }
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.FullSync:
                    syncsFlow.state = fullSync;
                    break;
                case SyncsType.Types.SelectiveSync:
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


