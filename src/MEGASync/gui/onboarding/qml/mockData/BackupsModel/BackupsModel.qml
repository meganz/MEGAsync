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

    function insert(folder) {
        console.debug("mockup BackupsModel -> insert() : folder -> " + folder);
    }

    function check() {
        console.debug("mockup BackupsModel -> check()");
    }

    function rename(folder, name) {
        console.debug("mockup BackupsModel -> rename() : folder -> " + folder
                      + " : name -> " + name);
    }

    function remove(folder) {
        console.debug("mockup BackupsModel -> remove() : folder -> " + folder);
    }

    function change(oldFolder, newFolder) {
        console.debug("mockup BackupsModel -> change() : oldFolder -> " + oldFolder
                      + " : newFolder -> " + newFolder);
    }

    signal backupsCreationFinished()
    signal checkAllStateChanged()
    signal existConfilctsChanged()
    signal noneSelected()

    property string mTotalSize: "24.7 MB"
    property int mCheckAllState: Qt.Unchecked
    property bool mExistConflicts: true
    property string mConflictsNotificationText: "This is an error text"

    ListElement {
        mName: "Desktop"
        mFolder: "C:\\Users\\mega\\Desktop"
        mSize: "30 MB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 1
    }
    ListElement {
        mName: "Documents12345678910111213141516171819202122232425262728293031323334353637383940"
        mFolder: "C:\\Users\\mega\\Documents12345678910111213141516171819202122232425262728293031323334353637383940"
        mSize: "2.3 GB12345678910111213141516171819202122232425262728293031323334353637383940"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 1
    }
    ListElement {
        mName: "Music"
        mFolder: "C:\\Users\\mega\\Music"
        mSize: "24.692 KB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
    }
    ListElement {
        mName: "Images"
        mFolder: "C:\\Users\\mega\\Images"
        mSize: "1 KB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
    }
}
