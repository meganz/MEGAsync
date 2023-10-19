// Local
import Onboard.Syncs_types 1.0

SyncTypePageForm {
    id: root

    signal moveToBack;
    signal moveToFullSync;
    signal moveToSelectiveSync;

    footerButtons {

        rightSecondary.onClicked: {
            console.log("rightSecondary clicked")
            root.moveToBack()
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.FullSync:
                    root.moveToFullSync()
                    break;
                case SyncsType.Types.SelectiveSync:
                    root.moveToSelectiveSync()
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


