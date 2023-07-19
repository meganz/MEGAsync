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
        mFolder: "C:\\Users\\mega\\Desktop"
        mSize: "30 MB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
    }

    ListElement {
        mName: "Documents12345678910111213141516171819202122232425262728293031323334353637383940"
        mFolder: "C:\\Users\\mega\\Documents12345678910111213141516171819202122232425262728293031323334353637383940"
        mSize: "2.3 GB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
    }
}
