pragma Singleton
import QtQuick 2.0

ListModel {
    id: proxyModel

    enum BackupErrorCode {
        None = 0,
        DuplicatedName = 1,
        ExistsRemote = 2
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

    property string mTotalSize: "24.7 MB"
    property int mCheckAllState: Qt.Unchecked
    property bool mExistConflicts: false

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
    ListElement {
        mName: "Music"
        toolTip: "C:\\Users\\mega\\Music"
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
        toolTip: "C:\\Users\\mega\\Images"
        mFolder: "C:\\Users\\mega\\Images"
        mSize: "1 KB"
        mSelected: false
        mSelectable: true
        mDone: false
        mError: 0
        mErrorVisible: false
    }
}
