import QtQuick 2.0
import QtQuick 2.15
import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 2.15 as Qml
import QtQuick.Controls 2.12 as QmlControlsv212
import QtQuick.Window 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.menus 1.0
import components.buttons 1.0 as Buttons
import components.dialogs 1.0
import components.toolTips 1.0

import SyncStatus 1.0
import QmlSyncType 1.0
import SyncModel 1.0
import AppStatsEvents 1.0

Item {
    id: root

    readonly property int headerHeight: 28
    readonly property int dividerMargins: 4
    readonly property int typeIconSize: 48
    readonly property int statusIconSize: 16
    readonly property int defaultColumnWidth: 100
    readonly property int lastColumnWidth: 84
    readonly property int tooltipMargin: 8
    property string dateFormat: "d MMM yyyy"
    property var locale: Qt.locale()

    property int oldWidth: width

    ErrorDialog {
        id: errorDialog

        acceptButtonText: Strings.ok
        visible: false
    }

    function getStatusText(model) {
        var result = "";
        if (model) {
            switch(model.status) {
            case SyncStatus.PAUSED:
                result = DeviceCentreStrings.statusPaused;
                break;
            case SyncStatus.STOPPED:
                result = DeviceCentreStrings.statusStopped + ".";
                break;
            case SyncStatus.UPDATING:
                result = DeviceCentreStrings.statusUpdating;
                break;
            case SyncStatus.UP_TO_DATE:
                result = DeviceCentreStrings.statusUpToDate;
                break;
            }
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

    function getPauseResumeActionText(currentStatus) {
        switch(currentStatus) {
        case SyncStatus.PAUSED:
            return DeviceCentreStrings.actionResume;
        case SyncStatus.STOPPED:
            return DeviceCentreStrings.actionResume;
        case SyncStatus.UPDATING:
            return DeviceCentreStrings.actionPause;
        case SyncStatus.UP_TO_DATE:
            return DeviceCentreStrings.actionPause;
        }
        return "";
    }

    function getPauseResumeActionIcon(currentStatus) {
        switch(currentStatus) {
        case SyncStatus.PAUSED:
            return Images.playCircle;
        case SyncStatus.STOPPED:
            return Images.playCircle;
        case SyncStatus.UPDATING:
            return Images.pauseCircle;
        case SyncStatus.UP_TO_DATE:
            return Images.pauseCircle;
        }
        return "";
    }

    function getRemoveActionText(currentType) {
        return (currentType === QmlSyncType.SYNC) ? DeviceCentreStrings.actionRemoveSync
                                                  : DeviceCentreStrings.actionStopBackup;
    }

    function getShowLocalText() {
        if (OS.isWindows()) {
            return DeviceCentreStrings.actionShowExplorer;
        }
        else if (OS.isMac()) {
            return DeviceCentreStrings.actionShowFinder;
        }
        else {
            return DeviceCentreStrings.actionShowFolder;
        }
    }

    function isErrorLinkVisible(model) {
        if (model && model.status === SyncStatus.STOPPED)
        {
            return (model.errorMessage !== "");
        }
        return false;
    }

    function getTooltipText(model) {
        if (model) {
            let message = DeviceCentreStrings.localPathLabel.arg(model.localPath) + "\n";
            message += DeviceCentreStrings.remotePathLabel.arg(model.remotePath);
            return message;
        }
        return "";
    }

    function getTooltipWidth(tooltipControl)
    {
        let charSize = tooltipControl.font.pixelSize;
        let tooltips = tooltipControl.text.split("\n");
        let maxCharCount = 0;
        for (let i = 0; i < tooltips.length; i++) {
            if (tooltips[i].length > maxCharCount) {
                maxCharCount = tooltips[i].length;
            }
        }
        let resizeFactor = 0.65;
        let margin = 2;
        return (maxCharCount * charSize) * resizeFactor + margin;
    }

    function getRebootActionText(currentType) {
        return (currentType === QmlSyncType.SYNC) ? DeviceCentreStrings.actionRebootSync
                                                  : DeviceCentreStrings.actionRebootBackup;
    }

    function getRebootActionWarningTitle(currentType) {
        return (currentType === QmlSyncType.SYNC) ? DeviceCentreStrings.rebootDialogTitleSync
                                                  : DeviceCentreStrings.rebootDialogTitleBackup;
    }

    function getRebootActionWarningBody(currentType) {
        return (currentType === QmlSyncType.SYNC) ? DeviceCentreStrings.rebootDialogBodySync.replace("[Br]","<br><br>")
                                                  : DeviceCentreStrings.rebootDialogBodyBackup.replace("[Br]","<br><br>");
    }
    anchors.fill: parent

    TableView {
        id: tableView

        anchors.fill: parent
        anchors.bottomMargin: 20
        model: deviceCentreAccess ? deviceCentreAccess.getSyncModel() : null

        style: TableViewStyle {
            id: tableViewStyle

            headerDelegate: Rectangle {
                id: headercomponent

                color: ColorTheme.pageBackground
                height: root.headerHeight

                Texts.Text {
                    id: textHeaderItem

                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: styleData.column === 0? 64 : 0
                    }

                    text: styleData.value
                    horizontalAlignment: Text.AlignLeft

                    font {
                        pixelSize: Texts.Text.Size.SMALL
                        weight: Font.DemiBold
                    }
                }
            }

            frame: Rectangle{ } // Make the table borderless
        }

        rowDelegate: Item {
            id: rowDelegateItem

            height: 64
            width: parent.width

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.DEVICE_CENTRE_SYNC_ROW_DOUBLE_CLICKED);
                    deviceCentreAccess.showInFolder(model.index);
                }
            }
        }

        TableViewColumn {
            id: nameColumn

            role: "name"
            title: DeviceCentreStrings.name
            width: 382
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
                            return model.type === QmlSyncType.BACKUP? Images.backupFolder : Images.syncFolder
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
                    color: ColorTheme.textPrimary
                    text: model ? model.name : ""
                }

                SvgImage {
                    id: statusImage

                    anchors.left: folderName.left
                    anchors.top: folderName.bottom
                    anchors.topMargin: 2
                    visible: true
                    source: model ? getStatusIcon(model.status) : ""
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
                    color: isErrorLinkVisible(model) ? ColorTheme.textError : ColorTheme.textPrimary
                    lineHeight: 18
                    text: getStatusText(model)
                }

                Texts.Text {
                    id: viewText

                    anchors {
                        left: statusText.right
                        top: statusText.top
                        leftMargin: 4
                    }
                    font {
                        pixelSize: Texts.Text.Size.NORMAL
                        weight: Font.Normal
                        underline: true
                    }
                    color: ColorTheme.textError
                    lineHeight: 18
                    text: DeviceCentreStrings.view
                    visible: isErrorLinkVisible(model)
                }

                MouseArea {
                    id: nameArea

                    hoverEnabled: true
                    anchors.fill: nameDelegateItem

                    ToolTip {
                        id: nameTooltip

                        visible: model && parent.containsMouse
                        text: getTooltipText(model)
                        delay: 500

                        onVisibleChanged: {
                            if (visible) {
                                x = parent.mouseX + tooltipMargin
                                y = parent.mouseY + tooltipMargin
                                width = getTooltipWidth(nameTooltip)
                            }
                        }
                    }
                }

                MouseArea{
                    id: viewClickArea

                    visible: viewText.visible
                    anchors.fill: viewText
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        errorDialog.titleText = model.errorMessage;
                        errorDialog.visible = true;
                        return;
                    }
                }
            }
        } // nameColumn

        TableViewColumn {
            id: typeColumn

            role: "type"
            title: DeviceCentreStrings.type
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
                        return model.type === QmlSyncType.BACKUP?  DeviceCentreStrings.backup : DeviceCentreStrings.sync
                    }
                    return "";
                }
            }
        } // typeColumn

        TableViewColumn {
            id: sizeColumn

            role: "size"
            title: DeviceCentreStrings.size
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
            id: dateModified

            role: "dateModified"
            title: DeviceCentreStrings.lastUpdated
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

            width: tableView.width - nameColumn.width - typeColumn.width - sizeColumn.width - dateModified.width - 20
            movable: false
            resizable: false

            delegate: Rectangle {
                id: contextMenuColumnDelegate

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
                    sizes.iconWidth: 16

                    onClicked: {
                        let buttonX = menuButton.mapToItem(null, 0, 0).x;
                        let buttonY = menuButton.mapToItem(null, 0, 0).y;
                        let menuHeight = menu.height;
                        let menuWidth = menu.width;
                        let xOffset = menuButton.width/2;
                        let yOffset = menuButton.height/2;
                        if (buttonX + menuWidth > Window.width) {
                            xOffset = xOffset - menuWidth; // Open left if close to right edge
                        }
                        if (buttonY + yOffset + menuHeight > Window.height) {
                            yOffset =  yOffset - menuHeight; // Open upwards if close to bottom edge
                        }
                        menu.popup(xOffset, yOffset);
                    }
                    
                    ContextMenu {
                        id: menu

                        onFocusChanged: {
                            if(menu.activeFocus === false){
                                menu.close();
                            }
                        }

                        ContextMenuItem {
                            id: openInMegaItem

                            text: DeviceCentreStrings.actionOpenInMega
                            icon.source: Images.megaOutline
                            onTriggered: {
                                deviceCentreAccess.openInMega(model.index);
                            }
                        }

                        ContextMenuItem {
                            id: showInFolder

                            text: getShowLocalText()
                            icon.source: Images.folderOpen
                            onTriggered: {
                                deviceCentreAccess.showInFolder(model.index);
                            }
                        }

                        QmlControlsv212.MenuSeparator {
                            padding: 0
                            topPadding: 4
                            bottomPadding: 4

                            contentItem: Rectangle {
                                id: separatorRect

                                implicitWidth: parent.width
                                implicitHeight: 1
                                color: ColorTheme.surface2
                            }
                        }

                        ContextMenuItem {
                            id: pauseItem

                            text: model? getPauseResumeActionText(model.status) : ""
                            icon.source: model? getPauseResumeActionIcon(model.status): ""
                            onTriggered: {
                                let PAUSE_SYNC = (model.status === SyncStatus.UPDATING || model.status === SyncStatus.UP_TO_DATE);
                                if(PAUSE_SYNC) {
                                    proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.DEVICE_CENTRE_PAUSE_ACTION_TIGGERED);
                                    deviceCentreAccess.pauseSync(model.index, PAUSE_SYNC);
                                }
                                else {
                                    proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.DEVICE_CENTRE_RESUME_ACTION_TRIGGERED);
                                    deviceCentreAccess.resumeSync(model.index, PAUSE_SYNC);
                                }
                            }
                        }

                        QmlControlsv212.MenuSeparator {
                            padding: 0
                            topPadding: 4
                            bottomPadding: 4

                            contentItem: Rectangle {
                                implicitWidth: parent.width
                                implicitHeight: 1
                                color: ColorTheme.surface2
                            }
                        }

                        ContextMenuItem {
                            id: manageExclusionsAction

                            text: DeviceCentreStrings.actionManageExclusions
                            icon.source: Images.slashCircle
                            onTriggered: {
                                deviceCentreAccess.manageExclusions(model.index);
                            }
                        }

                        QmlControlsv212.MenuSeparator {
                            padding: 0
                            topPadding: 4
                            bottomPadding: 4

                            contentItem: Rectangle {
                                implicitWidth: parent.width
                                implicitHeight: 1
                                color: ColorTheme.surface2
                            }
                        }

                        ContextMenuItem {
                            id: quickRescanAction

                            text: DeviceCentreStrings.actionQuickRescan
                            icon.source: Images.searchHollow
                            onTriggered: {
                                let DEEP_RESCAN = false;
                                deviceCentreAccess.rescanSync(model.index, DEEP_RESCAN);
                            }
                        }

                        ContextMenuItem {
                            id: deepRescanAction

                            text: DeviceCentreStrings.actionDeepRescan
                            icon.source: Images.searchFilled
                            onTriggered: {
                                let DEEP_RESCAN = true;
                                deviceCentreAccess.rescanSync(model.index, DEEP_RESCAN);
                            }
                        }

                        ContextMenuItem {
                            id: rebootAction

                            TitleBodyDialog {
                                id: rebootConfirmationDialog

                                acceptButtonText: Strings.continueText
                                cancelButtonText: Strings.cancel
                                titleText: model? root.getRebootActionWarningTitle(model.type) : ""
                                bodyText:  model? root.getRebootActionWarningBody(model.type) : ""
                                visible: false
                                onAccepted: {
                                    deviceCentreAccess.rebootSync(model.index)
                                }
                            }
                            text:  model? root.getRebootActionText(model.type) : ""
                            icon.source: Images.power
                            onTriggered: {
                                rebootConfirmationDialog.visible = true;
                            }
                        }
                        QmlControlsv212.MenuSeparator {
                            padding: 0
                            topPadding: 4
                            bottomPadding: 4

                            contentItem: Rectangle {
                                implicitWidth: parent.width
                                implicitHeight: 1
                                color: ColorTheme.surface2
                            }
                        }

                        ContextMenuItem {
                            id: stopBackupAction

                            text: model ? getRemoveActionText(model.type) : DeviceCentreStrings.actionStopBackup
                            icon.source: Images.minusCircle
                            onTriggered: {
                                proxyStatsEventHandlerAccess.sendTrackedEvent(AppStatsEvents.DEVICE_CENTRE_STOP_BACKUP_ACTION_TRIGGERED);
                                deviceCentreAccess.stopSync(model.index);
                            }
                        }
                    }
                }
            }
        }

        onWidthChanged: {
            let widthDifference = width - oldWidth
            nameColumn.width = nameColumn.width + widthDifference
            oldWidth = width
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
        color: ColorTheme.surface2
        radius: 1
        height: 1
    }
}
