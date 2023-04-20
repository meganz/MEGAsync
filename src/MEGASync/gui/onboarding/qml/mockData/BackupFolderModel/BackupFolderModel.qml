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

    function getTotalSize() {
        return "7.33 GB";
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

    function getDisplayName(index) {
        var folder = backupListModel.get(index).folder;
        return folder.substring(folder.lastIndexOf('\\') + 1);
    }

    signal rowSelectedChanged(bool selectedRow, bool selectedAll)

    ListElement {
        folder: "C:\\Users\\mega\\Desktop"
        selected: false
        size: "30 MB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
    ListElement {
        folder: "C:\\Users\\mega\\Documents"
        selected: false
        size: "2.3 GB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
    ListElement {
        folder: "C:\\Users\\mega\\Music"
        selected: false
        size: "783.4 KB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
    ListElement {
        folder: "C:\\Users\\mega\\Images"
        selected: false
        size: "1 KB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
}
