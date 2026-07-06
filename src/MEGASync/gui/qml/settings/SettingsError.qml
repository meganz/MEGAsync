import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0

import SyncErrors 1.0

/*
  Shared error panel shown when a sync/backup item is in a FAIL state. It maps
  each error code to a single action button, plus an always-present destructive
  remove button. SettingsListDelegate configures the type-specific
  texts/icons/actions via the properties below.
*/
ColumnLayout {
    id: root

    property var settingsAccess
    required property int errorId
    required property string errorMessage
    required property string localFolder
    required property int itemIndex

    // Type-specific configuration (defaults match the sync variant).
    property string enableText: SettingsStrings.solveIssueEnableSync
    property string startNewText: SettingsStrings.solveIssueStartNewSync
    property url startNewIcon: Images.sync_plus_small_thin_outline
    property string removeText: SettingsStrings.solveIssueRemoveSyncedFolder
    property url removeIcon: Images.trash_small_thin_outline
    property var removeAction: function() { settingsAccess.removeNonConfirmation(itemIndex); }
    property bool showRestore: true
    // Vertical padding for the (non-destructive) action buttons. Defaults to the
    // SmallSizes default so the sync variant is unchanged; backup overrides it.
    property int buttonVerticalPadding: defaultButtonSizes.verticalPadding

    readonly property int issueHandlerSpacing: 4
    readonly property int topErrorLabelMargin: 8
    readonly property int errorLabelMargin: 10
    readonly property int issueLabelPixelSize: 12
    readonly property int buttonActionSpacing: 0
    readonly property int buttonFocusBorder: 4
    readonly property int timeToResetActionButtonState: 5000
    readonly property int buttonIconbusyAnimationDurationTime: 1000

    // Exposed as a plain property (not a binding) so external items can size
    // themselves from this value without binding to implicitHeight directly.
    // Binding to ColumnLayout.implicitHeight from outside the component causes
    // Qt 5.15's layout engine to emit implicitHeightChanged during its own
    // evaluation, which Qt detects as a binding loop.
    property real contentHeight: 0
    Component.onCompleted: contentHeight = implicitHeight
    onImplicitHeightChanged: contentHeight = implicitHeight

    width: parent ? parent.width : implicitWidth
    height: implicitHeight
    spacing: issueHandlerSpacing

    // Non-visual reference used only to read the default vertical padding.
    SmallSizes { id: defaultButtonSizes }

    Text {
        id: errorLabel

        Layout.topMargin: root.topErrorLabelMargin
        Layout.rightMargin: root.errorLabelMargin
        Layout.leftMargin: root.errorLabelMargin
        Layout.bottomMargin: root.issueHandlerSpacing
        Layout.fillWidth: true
        text: root.errorMessage
        font.pixelSize: root.issueLabelPixelSize
        font.weight: Font.Normal
        elide: Text.ElideRight
        color: ColorTheme.textPrimary
    }

    RowLayout {
        id: buttonList

        spacing: root.buttonActionSpacing
        Layout.bottomMargin: root.errorLabelMargin - buttonFocusBorder
        Layout.rightMargin: root.errorLabelMargin
        Layout.leftMargin: Layout.bottomMargin

        ErrorActionButton {
            id: retryButton

            errorId: root.errorId
            alwaysVisible: true
            spins: true
            resetInterval: root.timeToResetActionButtonState
            spinDuration: root.buttonIconbusyAnimationDurationTime
            sizes: SmallSizes { verticalPadding: root.buttonVerticalPadding }
            icons.source: Images.rotate_cw_small_thin_outline
            text: SettingsStrings.solveIssueButtonRetry
            onActionTriggered: settingsAccess.resume(itemIndex)
        }

        ErrorActionButton {
            id: getMoreStorageButton

            errorId: root.errorId
            triggerErrors: [SyncErrors.STORAGE_OVERQUOTA, SyncErrors.ACCOUNT_EXPIRED]
            resetInterval: root.timeToResetActionButtonState
            sizes: SmallSizes { verticalPadding: root.buttonVerticalPadding }
            text: SettingsStrings.solveIssueGetMoreStorage
            onActionTriggered: settingsAccess.onGetMoreQuotaClicked()
        }

        ErrorActionButton {
            id: enableButton

            errorId: root.errorId
            triggerErrors: [SyncErrors.LOGGED_OUT]
            spins: true
            resetInterval: root.timeToResetActionButtonState
            spinDuration: root.buttonIconbusyAnimationDurationTime
            sizes: SmallSizes { verticalPadding: root.buttonVerticalPadding }
            icons.source: Images.power_small_thin_outline
            text: root.enableText
            onActionTriggered: settingsAccess.resume(itemIndex)
        }

        ErrorActionButton {
            id: restoreButton

            errorId: root.errorId
            enabledForType: root.showRestore
            triggerErrors: [SyncErrors.REMOTE_NODE_INSIDE_RUBBISH, SyncErrors.REMOTE_NODE_MOVED_TO_RUBBISH]
            resetInterval: root.timeToResetActionButtonState
            sizes: SmallSizes { verticalPadding: root.buttonVerticalPadding }
            icons.source: Images.trash_off_small_thin_outline
            text: SettingsStrings.solveIssueRestoreFolder
            onActionTriggered: settingsAccess.restoreSyncedFolder(itemIndex)
        }

        ErrorActionButton {
            id: startNewButton

            errorId: root.errorId
            triggerErrors: [SyncErrors.LOCAL_FILESYSTEM_MISMATCH]
            spins: true
            resetInterval: root.timeToResetActionButtonState
            spinDuration: root.buttonIconbusyAnimationDurationTime
            sizes: SmallSizes { verticalPadding: root.buttonVerticalPadding }
            icons.source: root.startNewIcon
            text: root.startNewText
            onActionTriggered: settingsAccess.addItem()
        }

        ErrorActionButton {
            id: removeButton

            alwaysVisible: true
            resetInterval: root.timeToResetActionButtonState
            colors.background: ColorTheme.buttonError
            colors.pressed: ColorTheme.buttonErrorPressed
            colors.hover: ColorTheme.buttonErrorHover
            sizes: SmallSizes {}
            text: root.removeText
            colors.text: ColorTheme.textOnColor
            colors.textHover: ColorTheme.textOnColor
            colors.textPressed: ColorTheme.textOnColor
            icons.source: root.removeIcon
            icons.colorEnabled: ColorTheme.textOnColor
            icons.colorHovered: ColorTheme.textOnColor
            icons.colorPressed: ColorTheme.textOnColor
            onActionTriggered: root.removeAction()
        }
    }
}
