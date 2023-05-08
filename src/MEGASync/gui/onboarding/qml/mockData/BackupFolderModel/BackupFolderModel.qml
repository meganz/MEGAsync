import QtQuick 2.0

ListModel {
    id: backupListModel

    function insertFolder(folder) {
        backupListModel.insert(0, { folder: folder, selected: true, size: "24.5 TB" });
        rowSelectedChanged();
    }

    function setAllSelected(checked) {
        for (let i = 0; i < backupListModel.count; i++) {
            backupListModel.get(i).selected = checked;
        }
    }

    function getNumSelectedRows() {
        var total = 0;
        for (let i = 0; i < backupListModel.count; i++) {
            if(backupListModel.get(i).selected) {
                total++;
            }
        }
        return total;
    }

    function getTooltipText(index) {
        return "This is a text explanation, backup is not selectable..."
    }

    function updateConfirmed() {
        for (let i = 0; i < backupListModel.count; i++) {
            backupListModel.get(i).confirmed = backupListModel.get(i).selected;
        }
    }

    function getConfirmedDirs() {
        var dirs = [];
        for (let i = 0; i < backupListModel.count; i++) {
            if(backupListModel.get(i).confirmed) {
                dirs.push(backupListModel.get(i).folder);
            }
        }
        return dirs;
    }

    function update(path, errorCode) {
        for (let i = 0; i < backupListModel.count; i++) {
            if(backupListModel.get(i).folder === path) {
                if(errorCode === 0) {
                    backupListModel.get(i).done = true;
                } else {
                    backupListModel.get(i).error = errorCode;
                }
            }
        }
    }

    signal rowSelectedChanged(bool selectedRow, bool selectedAll)

    property string totalSize: "24.7 MB"

    ListElement {
        folder: "C:\\Users\\mega\\Desktop"
        display: "Desktop"
        toolTip: "C:\\Users\\mega\\Desktop"
        selected: false
        size: "30 MB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
    ListElement {
        folder: "C:\\Users\\mega\\Documents"
        display: "Documents"
        toolTip: "C:\\Users\\mega\\Documents"
        selected: false
        size: "2.3 GB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
    ListElement {
        folder: "C:\\Users\\mega\\Music"
        display: "Music"
        toolTip: "You can't backup this folder as it contains backed up folders."
        selected: false
        size: "783.4 KB"
        selectable: false
        confirmed: false
        done: false
        error: -14
    }
    ListElement {
        folder: "C:\\Users\\mega\\Images"
        display: "Images"
        toolTip: "C:\\Users\\mega\\Images"
        selected: false
        size: "1 KB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
}
