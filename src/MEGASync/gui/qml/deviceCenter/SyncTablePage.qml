import QtQuick 2.0
import QtQuick 2.15
import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import components.texts 1.0 as Texts

Item {
    id: root

    readonly property int headerHeight: 28
    readonly property int dividerMargins: 4

    anchors.fill: parent
    // Keeping this only for quick testing with QML preview
    ListModel {
        id: mModel
        ListElement {
            name: "Folder-1"
            type: "Sync"
            size: "10.5 GB"
            dateAdded: "5 Jan 2024"
            dateModified: "10 Jan 2024"
        }
        ListElement {
            name: "Folder-2"
            type: "Backup"
            size: "12.2 GB"
            dateAdded: "15 Jan 2024"
            dateModified: "20 Jan 2024"
        }
        ListElement {
            name: "Folder-3"
            type: "Sync"
            size: "3.1 MB"
            dateAdded: "1 Feb 2024"
            dateModified: "1 March 2024"
        }
        ListElement {
            name: "Folder-1"
            type: "Backup"
            size: "10.5 GB"
            dateAdded: "1 April 2024"
            dateModified: "16 April 2024"
        }
        ListElement {
            name: "Folder-2"
            type: "Backup"
            size: "12.2 GB"
            dateAdded: "1 Jan 2024"
            dateModified: "1 Jan 2024"
        }
        ListElement {
            name: "Folder-3"
            type: "Backup"
            size: "3.1 MB"
            dateAdded: "1 Jan 2024"
            dateModified: "1 Jan 2024"
        }
    }

    TableView {
        id: tableView

        anchors.fill: parent
        anchors.bottomMargin: 20
        model: deviceCenterAccess.syncModel //mModel
        alternatingRowColors: false

        style: TableViewStyle {
            headerDelegate: Item {
                id: headercomponent

                height: root.headerHeight
                Text {
                    text: styleData.value
                    anchors.centerIn: parent
                }
                Rectangle{
                    id: headerDivider

                    anchors{
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                        leftMargin: styleData.column === 0? root.dividerMargins : 0
                        rightMargin: styleData.column === control.columnCount - 1? root.dividerMargins : 0
                    }
                    color: colorStyle.divider
                    radius: 1
                    height: 1

                }
            }
            itemDelegate: Rectangle {
                color: styleData.selected ? "lightgray" : "white"
                Text {
                    text: styleData.value
                    anchors.centerIn: parent
                }
            }
             frame: Rectangle{ } // Make the table borderless
        }
        rowDelegate: Item {
            height: 64
            width: parent.width
        }
        TableViewColumn {
            role: "name"
            title: "Name"
            width: 120
        }

        TableViewColumn {
            role: "type"
            title: "Type"
            width: 100
        }

        TableViewColumn {
            role: "size"
            title: "Size"
            width: 100
        }

        TableViewColumn {
            role: "dateAdded"
            title: "Date Added"
            width: 100
        }

        TableViewColumn {
            role: "dateModified"
            title: "Last Updated"
            width: 70
        }
    }
}
