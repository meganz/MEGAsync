import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

import Common 1.0
import Components 1.0 as Custom
import Onboard.Syncs_types 1.0

SelectFoldersPageForm {

    onVisibleChanged: {
        if(visible) {
            backupTable.backupProxyModel.selectedFilterEnabled = false;
        }
    }

    addFoldersMouseArea.onClicked: {
        folderDialog.visible = true;
    }

    FileDialog {
        id: folderDialog

        title: "";
        folder: shortcuts.home;
        selectFolder: true
        onAccepted: {
            var processedFolder = folder.toString().substring(8);
            backupTable.backupModel.insertFolder(processedFolder);
        }
    }

}
