import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncTypePageForm {

    buttonGroup.onCheckedButtonChanged: {
        switch(buttonGroup.checkedButton.type)
        {
        case ResumeButton.Type.FullSync:
            next = fullSync;
            break;
        case ResumeButton.Type.SelectiveSync:
            next = selectiveSync;
            break;
        }
        footerLayout.nextButton.enabled = next !== undefined;
    }

    onVisibleChanged: {
        console.log(buttonGroup.checkedButton);
        if(visible && buttonGroup.checkedButton !== undefined)
        {
            footerLayout.nextButton.enabled = false;
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
