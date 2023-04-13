import QtQuick 2.0

import BackupFolderModel 1.0

ListModel {

    property var sourceModel: BackupFolderModel
    property bool selectedFilterEnabled: false

    ListElement {
        folder: "C:/Users/mega/Documents"
        selected: true
        size: "30 MB"
        selectable: true
    }
    ListElement {
        folder: "C:/Users/mega/Images"
        selected: true
        size: "2.3 GB"
        selectable: true
    }
    ListElement {
        folder: "C:/Users/mega/Videos"
        selected: true
        size: "783.4 KB"
        selectable: true
    }
    ListElement {
        folder: "C:/Users/mega/Videos4"
        selected: true
        size: "4 KB"
        selectable: true
    }
    ListElement {
        folder: "C:/Users/mega/Videos5"
        selected: true
        size: "5 KB"
        selectable: true
    }
}
