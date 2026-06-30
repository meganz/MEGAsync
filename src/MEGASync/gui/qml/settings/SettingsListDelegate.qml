import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0
import components.toolTips 1.0

import SettingsModel 1.0

/*
  Shared list row for the Syncs and Backups settings tabs: name + folder-search icon,
  status cell, 3-dot context menu and (when failed) an embedded error component.

  Reads the model roles (name, folder, status, error, error_id, index) from the
  delegate context. Cosmetic and behavioural differences between the two tabs are
  injected through the properties below; everything else is common.
*/
Rectangle {
    id: root

    // --- model roles ---------------------------------------------------------------
    required property int status
    required property string name
    required property string folder
    required property string error
    required property int error_id
    required property int index

    // --- injected per-tab configuration -------------------------------------------
    // The owning tab widget (SyncSettingsQuickWidget / BackupSettingsQuickWidget).
    // Passed in explicitly because both widgets share one QML engine root context and
    // therefore cannot be exposed under a single global context-property name.
    property var settingsAccess

    property int statusContentWidth: 160
    property int itemRadius: 6
    property int errorRadius: 6
    // status cell text (per-tab strings)
    property var statusDescription: function(status) { return ""; }

    // status cell colours (per-tab); default to the Syncs behaviour
    property var resolveStatusTextColor: function(status, hovered) {
        if (hovered && (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING)) {
            return ColorTheme.textPrimary;
        }
        else if (status === SettingsModel.FAIL) {
            return ColorTheme.textError;
        }
        else if (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING) {
            return ColorTheme.textSecondary;
        }
        return ColorTheme.textPrimary;
    }
    property var resolveStatusIconColor: function(status, hovered) {
        if (hovered && (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING)) {
            return ColorTheme.iconPrimary;
        }
        else if (status === SettingsModel.FAIL) {
            return ColorTheme.textError;
        }
        else if (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING) {
            return ColorTheme.iconSecondary;
        }
        return ColorTheme.iconPrimary;
    }

    // context-menu per-tab differences
    property url openInMegaIcon: Images.mega_medium_thin_outline
    property url exclusionsIcon: Images.file_ignore_small_thin_outline
    property url rebootIcon: Images.rotate_cw_small_thin_outline
    property string rebootText: SettingsStrings.menuActionsRebootSync
    property url removeIcon: Images.trash_small_thin_outline
    property string removeText: SettingsStrings.solveIssueRemoveSyncedFolder
    // Syncs shows "Remove synced folder" in destructive red; Backups shows
    // "Stop backup" using the menu's default colours.
    property bool removeIsDestructive: true
    property url idleIcon: Images.sync_01_small_thin_outline

    // error panel per-tab configuration (defaults match the sync variant)
    property bool errorShowRestore: true
    property bool errorRetryOnIgnoreFileError: true
    property string errorEnableText: SettingsStrings.solveIssueEnableSync
    property string errorStartNewText: SettingsStrings.solveIssueStartNewSync
    property url errorStartNewIcon: Images.sync_plus_small_thin_outline
    property bool errorRemoveNonConfirmation: true
    property int errorButtonVerticalPadding: 4

    // --- shared dimensions --------------------------------------------------------
    readonly property int itemBackgroundHeight: 32
    readonly property int itemHorizontalPadding: 12
    readonly property int errorItemHorizontalPadding: 10
    readonly property int contentSpacing: 4
    readonly property int iconSize: 16
    readonly property int errorBorders: 2
    readonly property int backgroundColorAnimationTime: 200
    readonly property int toolTipShowDelay: 500
    readonly property int loadingAnimationNumberOfFrames: 30
    property real scanFrame: 0

    width: ListView.view ? ListView.view.width : implicitWidth
    height: content.implicitHeight
    radius: itemRadius
    color: status === SettingsModel.FAIL ? ColorTheme.notificationError : "transparent"

    function getItemTextColor() {
        if (status === SettingsModel.FAIL) {
            return ColorTheme.textError;
        }
        else if (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING) {
            return ColorTheme.textSecondary;
        }
        return ColorTheme.textPrimary;
    }

    function getItemIconColor() {
        if (status === SettingsModel.FAIL) {
            return ColorTheme.textError;
        }
        else if (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING) {
            return ColorTheme.iconSecondary;
        }
        return ColorTheme.iconPrimary;
    }

    function getStatusIcon(status) {
        switch (status) {
            case SettingsModel.FAIL:
                return Images.alert_circle_small_thin_outline;
            case SettingsModel.SUSPENDED:
                return Images.pause_thin_small_thin_outline;
            default:
                return root.idleIcon;
        }
    }

    function getShowInFolderLiteral() {
        if (OS.isMac()) {
            return SettingsStrings.menuActionsShowInFinder;
        }
        else if (OS.isWindows()) {
            return SettingsStrings.menuActionsShowInFileExplorer;
        }
        else {
            return SettingsStrings.menuActionsShowInFolder;
        }
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: root.errorBorders

        Rectangle {
            id: item

            Layout.topMargin: status === SettingsModel.FAIL ? root.errorBorders : 0
            Layout.rightMargin: Layout.topMargin
            Layout.leftMargin: Layout.topMargin
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredHeight: root.itemBackgroundHeight
            color: itemContainsMouse || menu.visible ? ColorTheme.surface1 : ColorTheme.pageBackground
            radius: status === SettingsModel.FAIL ? root.errorRadius : root.itemRadius

            property bool itemContainsMouse: itemMouseArea.containsMouse || nameAndFolderSearchMouseArea.containsMouse ||
                                             menuButton.hovered || statusMouseArea.containsMouse ||
                                             nameMouseArea.containsMouse

            Behavior on color {
                ColorAnimation {
                    duration: root.backgroundColorAnimationTime
                }
            }

            MouseArea {
                id: itemMouseArea

                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton
                enabled: status !== SettingsModel.REMOVING

                onClicked: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        menu.popup(mouse.x, mouse.y)
                    }
                }
            }

            RowLayout {
                id: itemRow

                anchors.fill: parent
                anchors.leftMargin: (status === SettingsModel.FAIL) ? root.errorItemHorizontalPadding : root.itemHorizontalPadding
                anchors.rightMargin: anchors.leftMargin
                spacing: root.contentSpacing

                Item {
                    id: parentNameContent

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    RowLayout {
                        id: nameContent

                        anchors.fill: parent

                        RowLayout {
                            id: nameWrapper

                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: nameWrapper.implicitWidth
                            Layout.preferredHeight: parent.height
                            spacing: root.contentSpacing

                            Text {
                                id: itemName

                                text: name
                                Layout.alignment: Qt.AlignVCenter
                                wrapMode: Text.NoWrap
                                elide: Text.ElideRight
                                Layout.preferredWidth: Math.min(implicitWidth, nameContent.width - folderSearchIcon.width - 2 * root.contentSpacing)
                                color: nameAndFolderSearchMouseArea.containsMouse && (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING) ? ColorTheme.textPrimary : root.getItemTextColor()

                                ToolTip {
                                    visible: nameMouseArea.containsMouse && (itemName.implicitWidth > (nameContent.width - folderSearchIcon.width - 2 * root.contentSpacing))
                                    text: name
                                    delay: root.toolTipShowDelay
                                    maxWidth: nameContent.width - folderSearchIcon.width - 2 * root.contentSpacing
                                }

                                MouseArea {
                                    id: nameMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                }
                            }

                            SvgImage {
                                id: folderSearchIcon

                                color: nameAndFolderSearchMouseArea.containsMouse && (status === SettingsModel.SUSPENDED || status === SettingsModel.REMOVING) ? ColorTheme.iconPrimary : root.getItemIconColor()
                                source: Images.folder_search_small_thin_outline
                                sourceSize: Qt.size(root.iconSize, root.iconSize)
                                visible: item.itemContainsMouse
                                Layout.alignment: Qt.AlignVCenter

                                ToolTip {
                                    visible: nameAndFolderSearchMouseArea.containsMouse
                                    text: getShowInFolderLiteral()
                                    delay: root.toolTipShowDelay
                                }

                                MouseArea {
                                    id: nameAndFolderSearchMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        settingsAccess.exploreLocal(folder);
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }

                RowLayout {
                    id: statusContent

                    spacing: root.contentSpacing
                    Layout.preferredWidth: root.statusContentWidth
                    Layout.maximumWidth: root.statusContentWidth
                    Layout.preferredHeight: parent.height

                    Item {
                        id: statusWrapper

                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.height

                        RowLayout {
                            id: statusRowContent

                            anchors.verticalCenter: parent.verticalCenter

                            SvgImage {
                                id: statusIcon

                                Layout.alignment: Qt.AlignVCenter
                                color: root.resolveStatusIconColor(status, statusMouseArea.containsMouse)
                                source: root.getStatusIcon(status)
                                sourceSize: Qt.size(root.iconSize, root.iconSize)
                                visible: status !== SettingsModel.SCANNING && status !== SettingsModel.REMOVING
                            }

                            SvgImage {
                                id: scanningIcon

                                Layout.alignment: Qt.AlignVCenter
                                color: root.resolveStatusIconColor(status, statusMouseArea.containsMouse)
                                source: "qrc:/activity_indicator_" + (Math.floor(root.scanFrame) % root.loadingAnimationNumberOfFrames + 1) + ".svg"
                                sourceSize: Qt.size(root.iconSize, root.iconSize)
                                visible: status === SettingsModel.SCANNING || status === SettingsModel.REMOVING
                            }

                            NumberAnimation {
                                target: root
                                property: "scanFrame"
                                from: 0
                                to: root.loadingAnimationNumberOfFrames
                                duration: 1800
                                loops: Animation.Infinite
                                running: status === SettingsModel.SCANNING || status === SettingsModel.REMOVING
                                onStopped: root.scanFrame = 0
                            }

                            Text {
                                id: statusText

                                Layout.alignment: Qt.AlignVCenter
                                text: root.statusDescription(status)
                                color: root.resolveStatusTextColor(status, statusMouseArea.containsMouse)
                            }
                        }

                        MouseArea {
                            id: statusMouseArea

                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: (status === SettingsModel.SUSPENDED) ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: {
                                if (status === SettingsModel.SUSPENDED) {
                                    settingsAccess.resume(index);
                                }
                            }
                        }
                    }

                    IconButton {
                        id: menuButton

                        icons.source: Images.threeDots
                        icons.colorEnabled: status === SettingsModel.FAIL ? ColorTheme.textError : ColorTheme.iconPrimary
                        sizes {
                            iconWidth: root.iconSize
                            spacing: 0
                            textLineHeight: root.iconSize
                        }
                        visible: item.itemContainsMouse && status !== SettingsModel.REMOVING
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: {
                            menu.popup(item.width - menu.width, item.height)
                        }
                    }
                }
            }

            ContextMenu {
                id: menu

                onFocusChanged: {
                    if (menu.activeFocus === false) {
                        menu.close();
                    }
                }

                onVisibleChanged: settingsAccess.setContextMenuOpen(visible)

                Connections {
                    target: settingsAccess
                    function onCloseContextMenu() { menu.close() }
                }

                ContextMenuItem {
                    text: getShowInFolderLiteral()
                    icon.source: Images.folder_small_thin_outline
                    onTriggered: {
                        settingsAccess.exploreLocal(folder);
                    }
                }

                ContextMenuItem {
                    text: SettingsStrings.menuActionsOpenInMega
                    icon.source: root.openInMegaIcon
                    onTriggered: {
                        settingsAccess.openInMega(index);
                    }
                }

                MenuSeparator {
                }

                ContextMenuItem {
                    visible: (status === SettingsModel.IDLE || status === SettingsModel.SCANNING || status == SettingsModel.ACTIVE || status == SettingsModel.LOADING)
                    height: visible ? implicitHeight : 0
                    text: SettingsStrings.menuActionsPause
                    icon.source: Images.pause_thin_small_thin_outline
                    onTriggered: {
                        settingsAccess.pause(index);
                    }
                }

                ContextMenuItem {
                    visible: status === SettingsModel.SUSPENDED
                    height: visible ? implicitHeight : 0
                    text: SettingsStrings.menuActionsResume
                    icon.source: Images.play_small_thin_outline
                    onTriggered: {
                        settingsAccess.resume(index);
                    }
                }

                MenuSeparator {
                    visible: status === SettingsModel.IDLE || status === SettingsModel.SUSPENDED
                    height: visible ? implicitHeight : 0
                }

                ContextMenuItem {
                    text: SettingsStrings.menuActionsManageExclusions
                    icon.source: root.exclusionsIcon
                    onTriggered: {
                        settingsAccess.openExclusionsDialog(index);
                    }
                }

                MenuSeparator {
                }

                ContextMenuItem {
                    visible: status !== SettingsModel.FAIL && status !== SettingsModel.SUSPENDED
                    height: visible ? implicitHeight : 0
                    text: SettingsStrings.menuActionsRescan
                    icon.source: Images.search_large_small_thin_outline
                    onTriggered: {
                        settingsAccess.rescan(index);
                    }
                }

                ContextMenuItem {
                    visible: status !== SettingsModel.FAIL && status !== SettingsModel.SUSPENDED
                    height: visible ? implicitHeight : 0
                    text: root.rebootText
                    icon.source: root.rebootIcon
                    onTriggered: {
                        settingsAccess.reboot(index);
                    }
                }

                MenuSeparator {
                    visible: status !== SettingsModel.FAIL && status !== SettingsModel.SUSPENDED
                    height: visible ? implicitHeight : 0
                }

                ContextMenuItem {
                    id: removeMenuItem

                    text: root.removeText
                    textColor: root.removeIsDestructive ? ColorTheme.textError : colors.text
                    imageColor: root.removeIsDestructive ? ColorTheme.textError : colors.icon
                    icon.source: root.removeIcon
                    onTriggered: {
                        settingsAccess.remove(index);
                    }
                }
            }
        }

        Rectangle {
            id: errorItem

            visible: status === SettingsModel.FAIL
            Layout.bottomMargin: root.errorBorders
            Layout.rightMargin: Layout.bottomMargin
            Layout.leftMargin: Layout.bottomMargin
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredHeight: errorPanel.contentHeight
            color: ColorTheme.notificationError
            radius: root.errorRadius

            SettingsError {
                id: errorPanel

                width: parent.width
                settingsAccess: root.settingsAccess
                errorId: error_id
                errorMessage: error
                localFolder: folder
                itemIndex: index
                showRestore: root.errorShowRestore
                retryOnIgnoreFileError: root.errorRetryOnIgnoreFileError
                enableText: root.errorEnableText
                startNewText: root.errorStartNewText
                startNewIcon: root.errorStartNewIcon
                removeText: root.removeText
                removeIcon: root.removeIcon
                removeAction: root.errorRemoveNonConfirmation
                              ? function() { settingsAccess.removeNonConfirmation(itemIndex); }
                              : function() { settingsAccess.remove(itemIndex);}
                buttonVerticalPadding: root.errorButtonVerticalPadding
            }
        }
    }
}
