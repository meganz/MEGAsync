import QtQuick 2.12

ConfirmFoldersPageForm {

    onVisibleChanged: {
        if(visible) {
            backupTable.backupProxyModel.selectedFilterEnabled = true;
        }
    }

}
