import QtQuick 2.0

ListModel {
    id: backupListModel

    function insertFolder(folder) {
        backupListModel.insert(0, { folder: folder, selected: true, size: "24.5 TB" });
    }

    function setAllSelected(checked) {
        for (let i = 0; i < backupListModel.count; i++) {
            backupListModel.get(i).selected = checked;
        }
    }

    signal rowSelectedChanged
    signal allRowsSelected(bool selected)

    ListElement {
        folder: "C:/Users/mega/Documents"
        selected: false
        size: "30 MB"
    }
    ListElement {
        folder: "C:/Users/mega/Images"
        selected: false
        size: "2.3 GB"
    }
    ListElement {
        folder: "C:/Users/mega/Videos"
        selected: true
        size: "783.4 KB"
    }
    ListElement {
        folder: "C:/Users/mega/Videos1"
        selected: false
        size: "1 KB"
    }
    ListElement {
        folder: "C:/Users/mega/Videos2"
        selected: false
        size: "2 KB"
    }
    ListElement {
        folder: "C:/Users/mega/Videos3"
        selected: false
        size: "3 KB"
    }
    ListElement {
        folder: "C:/Users/mega/Videos4"
        selected: true
        size: "4 KB"
    }
    ListElement {
        folder: "C:/Users/mega/Videos5"
        selected: true
        size: "5 KB"
    }
}
