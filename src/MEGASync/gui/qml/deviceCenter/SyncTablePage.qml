import QtQuick 2.0
import QtQuick 2.15
import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import SyncModel 1.0

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0 as Buttons
import SyncStatus 1.0
import QmlSyncType 1.0


Item {
    id: root

    readonly property int headerHeight: 28
    readonly property int dividerMargins: 4
    readonly property int typeIconSize: 48
    readonly property int statusIconSize: 16
    readonly property int defaultColumnWidth: 100
    readonly property int lastColumnWidth: 84
    property string dateFormat: "d MMM yyyy"
    property var locale: Qt.locale()

    function getStatusText(currentStatus) {
        var result = "";
        switch(currentStatus) {
        case SyncStatus.PAUSED:
            result = DeviceCenterStrings.statusPaused;
            break;
        case SyncStatus.STOPPED:
            result = DeviceCenterStrings.statusStopped;
            break;
        case SyncStatus.UPDATING:
            result = DeviceCenterStrings.statusUpdating;
            break;
        case SyncStatus.UP_TO_DATE:
            result = DeviceCenterStrings.statusUpToDate;
            break;
        }
        return result;
    }
    function getStatusIcon(currentStatus) {
        var result = "";
        switch(currentStatus) {
        case SyncStatus.PAUSED:
            result = Images.statusPaused;
            break;
        case SyncStatus.STOPPED:
            result = Images.statusStopped;
            break;
        case SyncStatus.UPDATING:
            result = Images.statusUpdating;
            break;
        case SyncStatus.UP_TO_DATE:
            result = Images.statusUpToDate;
            break;
        }
        return result;
    }

    anchors.fill: parent
    TableView {
        id: tableView

        anchors.fill: parent
        anchors.bottomMargin: 20
        model: deviceCenterAccess.syncModel
        style: TableViewStyle {
            headerDelegate: Rectangle {
                id: headercomponent

                color: colorStyle.pageBackground
                height: root.headerHeight
                Texts.Text {
                    text: styleData.value
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: styleData.column === 0? 64 : 0
                    horizontalAlignment: Text.AlignLeft
                    font {
                        pixelSize: Texts.Text.Size.SMALL
                        weight: Font.DemiBold
                    }
                }
            }
            frame: Rectangle{ } // Make the table borderless
        } // TableViewStyle
        rowDelegate: Item {
            height: 64
            width: parent.width
        }
        TableViewColumn {
            id: nameColumn

            role: "name"
            title: DeviceCenterStrings.name
            width: 282
            movable: false
            resizable:false
            delegate: Rectangle {
                id: nameDelegateItem


                anchors.fill: parent
                SvgImage {
                    id: typeImage

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 28
                    anchors.topMargin: 10
                    visible: true
                    source: {
                        if(model) {
                            return model.type === SyncModel.BACKUP? Images.backupFolder : Images.syncFolder
                        }
                        return "";
                    }
                    height: root.typeIconSize
                    width: root.typeIconSize
                    sourceSize: Qt.size(36, 32)
                }
                Texts.ElidedText {
                    id: folderName

                    anchors{
                        left: typeImage.right
                        top: parent.top
                        right:parent.right
                        rightMargin: 2
                        leftMargin: 8
                        topMargin: 12
                    }
                    wrapMode: Text.NoWrap
                    elide: Qt.ElideMiddle
                    font {
                        pixelSize: Texts.Text.Size.MEDIUM
                        weight: Font.DemiBold
                    }
                    color: colorStyle.textPrimary
                    text: model? model.name : ""
                }
                SvgImage {
                    id: statusImage

                    anchors.left: folderName.left
                    anchors.top: folderName.bottom
                    anchors.topMargin: 2
                    visible: true
                    source: model? getStatusIcon(model.status) : ""
                    height: root.statusIconSize
                    width: root.statusIconSize
                    sourceSize: Qt.size(14.5, 14.5)
                }
                Texts.Text {
                    id: statusText

                    anchors {
                        left: statusImage.right
                        top: statusImage.top
                        leftMargin: 4
                    }
                    font {
                        pixelSize: Texts.Text.Size.NORMAL
                        weight: Font.Normal
                    }
                    lineHeight: 18
                    text: model? getStatusText(model.status) : ""
                }
            }
        } // nameColumn

        TableViewColumn {
            id: typeColumn

            role: "type"
            title: DeviceCenterStrings.type
            width: defaultColumnWidth
            movable: false
            resizable: false

            delegate: Texts.Text {
                id: typeText

                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: parent.verticalCenter
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.Normal
                }
                lineHeight: 18
                text: {
                    if(model) {
                        return model.type === QmlSyncType.BACKUP?  DeviceCenterStrings.backup : DeviceCenterStrings.sync
                    }
                    return "";
                }
            }
        } // typeColumn

        TableViewColumn {
            id: sizeColumn

            role: "size"
            title: DeviceCenterStrings.size
            width: defaultColumnWidth
            movable: false
            resizable: false
            delegate: Texts.Text {
                id: sizeText

                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: parent.verticalCenter
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.Normal
                }
                lineHeight: 18
                text: model? model.size : ""
            }
        } // sizeColumn

        TableViewColumn {
            id: dateAddedColumn

            role: "dateAdded"
            title: DeviceCenterStrings.dateAdded
            width: defaultColumnWidth
            movable: false
            resizable: false
            delegate: Texts.ElidedText {
                id: dateAddedText

                verticalAlignment: Text.AlignVCenter
                anchors {
                    verticalCenter: parent.verticalCenter
                }
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.Normal
                }
                lineHeight: 18
                text: model? model.dateAdded.toLocaleDateString(locale, dateFormat) : ""
            }
        } // dateAddedColumn

        TableViewColumn {
            id: dateModified

            role: "dateModified"
            title: DeviceCenterStrings.lastUpdated
            width: defaultColumnWidth
            movable: false
            resizable: false
            delegate: Texts.ElidedText {
                id: dateUpdatedText

                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: parent.verticalCenter
                font {
                    pixelSize: Texts.Text.Size.NORMAL
                    weight: Font.Normal
                }
                lineHeight: 18
                text: model? model.dateModified.toLocaleDateString(locale, dateFormat) : ""
            }
        }

        TableViewColumn {
            id: contextMenuColumn

            width: tableView.width - nameColumn.width - typeColumn.width - sizeColumn.width - dateAddedColumn.width - dateModified.width - 20
            movable: false
            resizable: false
            delegate: Rectangle {
                anchors.fill: parent
                Buttons.IconButton {
                    id: menuButton

                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 40
                    }
                    icons.source: Images.threeDots
                    visible: true
                    sizes{
                        iconWidth: 16
                    }
                }
            }
        }
    }
    Rectangle {
        id: headerDivider

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: root.headerHeight
            leftMargin: 4
            rightMargin: 4
        }
        color: colorStyle.divider
        radius: 1
        height: 1
    }
}
