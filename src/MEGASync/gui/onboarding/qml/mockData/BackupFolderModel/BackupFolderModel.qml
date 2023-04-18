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

    signal rowSelectedChanged(bool selectedRow, bool selectedAll)

    ListElement {
        folder: "C:\\Users\\mega\\Desktop"
        selected: false
        size: "30 MB"
        selectable: true
    }
    ListElement {
        folder: "C:\\Users\\mega\\Documents"
        selected: false
        size: "2.3 GB"
        selectable: true
    }
    ListElement {
        folder: "C:\\Users\\mega\\Music"
        selected: false
        size: "783.4 KB"
        selectable: true
    }
    ListElement {
        folder: "C:\\Users\\mega\\Images"
        selected: false
        size: "1 KB"
        selectable: true
    }
}
