import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncsPage {

    objectName: "SyncTypes"
    //substackView.initialItem: selectSyncType

    SyncTypePage{
        id: selectSyncType
       // footerLayout: mainItem.footerLayout
    }

    FullSyncPage
    {
        id: fullSync
        previous: selectSyncType
       // footerLayout: mainItem.footerLayout
        visible: false
    }

    SelectivePage
    {
        id: selectiveSync
        previous: selectSyncType
       // footerLayout: mainItem.footerLayout
        visible: false
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
