import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom
import Onboard.Syncs_types.Backups 1.0
//import Onboard.Syncs_types.Syncs 1.0

Item {
    id: mainItem

//    function showSubstackPage(type) {
//        var item;
//        switch(type) {
//            case InstallationTypeButton.Type.Sync:
//                item = syncPage;
//                console.debug("TODO: Add Sync page");
//                break;
//            case InstallationTypeButton.Type.Backup:
//                item = backupPage;
//                break;
//            case InstallationTypeButton.Type.Fuse:
//                console.debug("TODO: Add Fuse page");
//                break;
//            default:
//                console.error("Undefined option clicked -> " + option);
//                return;
//        }
//        item.resetSubstackToInitialPage();
//        if(substackView.currentItem !== item) {
//            substackView.replace(item, StackView.Immediate);
//        }
//    }

    objectName: "ContentPanel"
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
