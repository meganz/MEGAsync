import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import BackupSettingsModel 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 19
    readonly property int backupTableSpacing: 4
    readonly property int backupStatusLabelWidth: 172
    readonly property int listViewOfBackupItems: 270
    readonly property int errorBackupItemRadius: 4
    readonly property int backupItemContentStatusWidth: 172
    readonly property int folderSearchIconSize: 16
    readonly property int verticalAddBackupButtonSeparator: 16
    readonly property int leftPaddingAddBackupButton: 12
    readonly property int rightPaddingAddBackupButton: 16
    readonly property int backupFooterHorizontalMargin: 12
    readonly property int backupFooterBottomMargin: 12
    readonly property int backupFooterLabelSpacing: 4
    readonly property int backupFooterSpacing: 8
    readonly property int backupFooterFieldHeight: 36
    readonly property int backupFooterFieldRadius: 8
    readonly property int backupFooterFieldHorizontalPadding: 10
    readonly property int backupFooterFieldSpacing: 6
    readonly property int backupFooterFieldTextPixelSize: 14
    readonly property int issueLabelPixelSize: 12
    readonly property int errorButtonVerticalPad: 3

    function getStatusDescription(status) {
        switch(status) {
            case BackupSettingsModel.PENDING:
            case BackupSettingsModel.LOADING:
                return SettingsStrings.syncStateLoading;
            case BackupSettingsModel.SUSPENDED:
                return SettingsStrings.syncStatePaused;
            case BackupSettingsModel.FAIL:
                return SettingsStrings.syncStateDisabled;
            case BackupSettingsModel.SCANNING:
                return SettingsStrings.syncStateScanning;
            case BackupSettingsModel.ACTIVE:
                return SettingsStrings.syncStateSyncing;
            case BackupSettingsModel.IDLE:
                return SettingsStrings.backupStateBackedUp;
            case BackupSettingsModel.REMOVING:
                return SettingsStrings.syncStateRemoving;
        }
    }

    function backupStatusTextColor(status, hovered) {
        if (hovered && (status === BackupSettingsModel.SUSPENDED || status === BackupSettingsModel.REMOVING)) {
            return ColorTheme.textPrimary;
        }
        else if (status === BackupSettingsModel.FAIL) {
            return ColorTheme.textError;
        }
        return ColorTheme.textPrimary;
    }

    function backupStatusIconColor(status, hovered) {
        if (hovered && (status === BackupSettingsModel.SUSPENDED || status === BackupSettingsModel.REMOVING)) {
            return ColorTheme.iconPrimary;
        }
        else if (status === BackupSettingsModel.FAIL) {
            return ColorTheme.textError;
        }
        return ColorTheme.iconSecondary;
    }

    SettingsEmptyState {
        visible: backupList.count === 0
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        image: Images.backupDevices
        title: SettingsStrings.titleNoBackup
        description: SettingsStrings.descriptionNoBackup
        buttonText: SettingsStrings.addBackup
        onAddClicked: backupSettingsAccess.addItem()
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
        spacing: backupTableSpacing
        visible: backupList.count > 0

        SettingsSortableHeader {
            Layout.preferredWidth: parent.width
            nameText: SettingsStrings.tableBackupsNameColumn
            statusText: SettingsStrings.tableSyncsStatusColumn
            labelColor: ColorTheme.textAccent
            showInitialNameIndicator: false
            statusLabelWidth: root.backupStatusLabelWidth
            onSortByName: backupSettingsAccess.sortModelByName(ascending)
            onSortByStatus: backupSettingsAccess.sortModelByStatus(ascending)
        }

        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        ListView {
            id: backupList

            Layout.preferredWidth: parent.width
            Layout.minimumHeight: listViewOfBackupItems
            Layout.fillHeight: true
            Layout.maximumHeight: Math.max(listViewOfBackupItems, contentHeight)

            model: backupSettingsModel
            spacing: backupTableSpacing
            interactive: contentHeight > height
            clip: true

            delegate: SettingsListDelegate {
                settingsAccess: backupSettingsAccess
                statusContentWidth: root.backupItemContentStatusWidth
                errorRadius: root.errorBackupItemRadius
                idleIcon: Images.database_small_thin_outline
                errorShowRestore: false
                errorEnableText: SettingsStrings.solveIssueEnableBackup
                errorStartNewText: SettingsStrings.solveIssueStartNewBackup
                errorStartNewIcon: Images.database_plus_small_thin_outline
                errorRemoveNonConfirmation: false
                errorButtonVerticalPadding: root.errorButtonVerticalPad
                rebootText: SettingsStrings.menuActionsRebootBackup
                removeIcon: Images.database_x_medium_thin_outline
                removeText: SettingsStrings.menuActionsStopBackup
                statusDescription: root.getStatusDescription
                resolveStatusTextColor: root.backupStatusTextColor
                resolveStatusIconColor: root.backupStatusIconColor
            }

            ScrollBar.vertical: ScrollBar {
                policy: backupList.contentHeight > backupList.height
                      ? ScrollBar.AlwaysOn
                      : ScrollBar.AlwaysOff
            }
        }

        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        Item {
            Layout.preferredHeight: verticalAddBackupButtonSeparator
            Layout.preferredWidth: parent.width
        }

        Row {
            Layout.preferredWidth: parent.width
            layoutDirection: Qt.RightToLeft

            PrimaryButton {
                id: addBackupButton

                text: SettingsStrings.addBackup
                icons.source: Images.plus
                icons.position: Icon.Position.LEFT
                leftPadding: leftPaddingAddBackupButton
                rightPadding: rightPaddingAddBackupButton
                width: implicitWidth
                onClicked: {
                    backupSettingsAccess.addItem();
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width
        }

        ColumnLayout {
            id: backupFolderFooter

            Layout.fillWidth: true
            Layout.leftMargin: backupFooterHorizontalMargin
            Layout.rightMargin: backupFooterHorizontalMargin
            Layout.bottomMargin: backupFooterBottomMargin
            spacing: backupFooterLabelSpacing

            Text {
                id: backupFolderLabel

                text: SettingsStrings.backupFolderLabel
                color: ColorTheme.textPrimary
                font.pixelSize: root.issueLabelPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            RowLayout {
                id: backupFolderControls

                Layout.fillWidth: true
                spacing: backupFooterSpacing

                Rectangle {
                    id: backupFolderField

                    Layout.fillWidth: true
                    Layout.preferredHeight: backupFooterFieldHeight
                    Layout.alignment: Qt.AlignVCenter
                    color: ColorTheme.pageBackground
                    border.color: ColorTheme.borderStrongSelected
                    border.width: Constants.dividerThickness
                    radius: backupFooterFieldRadius

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: backupFooterFieldHorizontalPadding
                        anchors.rightMargin: backupFooterFieldHorizontalPadding
                        spacing: backupFooterFieldSpacing

                        SvgImage {
                            id: backupFolderIcon

                            source: Images.database_small_thin_outline
                            sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                            color: ColorTheme.iconPrimary
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Text {
                            id: backupFolderPath

                            text: backupSettingsAccess.backupFolderPath
                            color: ColorTheme.textPrimary
                            font.pixelSize: root.backupFooterFieldTextPixelSize
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                }

                SecondaryButton {
                    id: viewBackupFolderButton

                    text: SettingsStrings.viewInMega
                    enabled: backupSettingsAccess.backupFolderAvailable
                    Layout.alignment: Qt.AlignVCenter
                    onClicked: {
                        backupSettingsAccess.openBackupFolder();
                    }
                }
            }
        }
    }
}
