import QtQuick 2.0

import BackupFolderModel 1.0

ListModel {

    property var sourceModel: BackupFolderModel
    property bool selectedFilterEnabled: false

    ListElement {
        folder: "C:\\Users\\mega\\Desktop"
        display: "Desktop"
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
        selected: false
        size: "783.4 KB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
    ListElement {
        folder: "C:\\Users\\mega\\Images"
        display: "Images"
        selected: false
        size: "1 KB"
        selectable: true
        confirmed: false
        done: false
        error: 0
    }
}
