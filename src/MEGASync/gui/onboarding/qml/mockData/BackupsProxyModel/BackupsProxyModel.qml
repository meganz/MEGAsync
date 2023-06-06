import QtQuick 2.0

ListModel {
    id: proxyModel

    function createBackups() {
        console.log("mockup BackupsProxyModel::createBackups()");
        BackupsController.createBackups();
    }

    signal rowSelectedChanged(bool selectedRow, bool selectedAll)

    property bool selectedFilterEnabled: false

    onSelectedFilterEnabledChanged: {
        console.log("mockup BackupsProxyModel::selectedFilterEnabled: " + selectedFilterEnabled);
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
