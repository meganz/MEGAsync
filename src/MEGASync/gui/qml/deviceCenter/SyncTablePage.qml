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

    TableView {
        id: tableView

        anchors.fill: parent
        anchors.bottomMargin: 20
        model: deviceCenterAccess.syncModel
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
