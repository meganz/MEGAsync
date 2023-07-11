import QtQuick 2.0

import BackupsController 1.0

ListModel {
    id: proxyModel

    property bool selectedFilterEnabled: false

    signal rowSelectedChanged(bool selectedRow, bool selectedAll)

    function createBackups() {
        console.debug("mockup BackupsProxyModel::createBackups()");
        BackupsController.createBackups();
    }

    onSelectedFilterEnabledChanged: {
        console.debug("mockup BackupsProxyModel::selectedFilterEnabled: "
                      + selectedFilterEnabled);
    }

    ListElement {
        mName: "Desktop"
        toolTip: "C:\\Users\\mega\\Desktop"
        mFolder: "C:\\Users\\mega\\Desktop"
        mSize: "30 MB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
        mErrorVisible: false
    }

    ListElement {
        mName: "Documents"
        toolTip: "C:\\Users\\mega\\Documents"
        mFolder: "C:\\Users\\mega\\Documents"
        mSize: "2.3 GB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
        mErrorVisible: false
    }
}
