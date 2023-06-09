// Local
import Onboard.Syncs_types 1.0

SyncTypePageForm {

    footerButtons {

        previousButton.onClicked: {
            mainFlow.state = syncType;
        }

        nextButton.onClicked: {
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
            footerButtons.nextButton.enabled = true;
        }
    }

}


