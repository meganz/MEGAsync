// Local
import Onboard.Syncs_types 1.0

SyncTypePageForm {

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncType;
        }

        nextButton.enabled: false
        nextButton.onClicked: {
            switch(syncPage.buttonGroup.checkedButton.type) {
                case ResumeButton.Type.FullSync:
                    syncsFlow.state = fullSync;
                    break;
                case ResumeButton.Type.SelectiveSync:
                    syncsFlow.state = selectiveSync;
                    break;
                default:
                    console.error("Button type does not exist -> "
                                  + syncPage.buttonGroup.checkedButton.type);
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


