pragma Singleton
import QtQuick 2.0

ListModel {
    id: proxyModel

    enum BackupErrorCode {
        None = 0,
        DuplicatedName = 1,
        ExistsRemote = 2,
        SyncConflict = 3,
        PathRelation = 4
    }

    function insertFolder(folder) {
        console.log("mockup BackupsModel -> insertFolder() : folder -> " + folder);
    }

    function checkBackups() {
        console.log("mockup BackupsModel -> checkBackups()");
    }

    function renameBackup(folder, name) {
        console.log("mockup BackupsModel -> renameBackup() : folder -> " + folder + " : name -> " + name);
    }

    function remove(folder) {
        console.log("mockup BackupsModel -> remove() : folder -> " + folder);
    }

    signal backupsCreationFinished()
    signal checkAllStateChanged()
    signal existConfilctsChanged()
    signal noneSelected()

    property string mTotalSize: "24.7 MB"
    property int mCheckAllState: Qt.Unchecked
    property bool mExistConflicts: false
    property string mConflictsNotificationText: ""

    ListElement {
        mName: "Desktop"
        mFolder: "C:\\Users\\mega\\Desktop"
        mSize: "30 MB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 1
        mErrorVisible: false
    }
    ListElement {
        mName: "Documents12345678910111213141516171819202122232425262728293031323334353637383940"
        mFolder: "C:\\Users\\mega\\Documents12345678910111213141516171819202122232425262728293031323334353637383940"
        mSize: "2.3 GB12345678910111213141516171819202122232425262728293031323334353637383940"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 1
        mErrorVisible: false
    }
    ListElement {
        mName: "Music"
        mFolder: "C:\\Users\\mega\\Music"
        mSize: "24.692 KB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
        mErrorVisible: false
    }
    ListElement {
        mName: "Images"
        mFolder: "C:\\Users\\mega\\Images"
        mSize: "1 KB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
        mErrorVisible: false
    }
}
