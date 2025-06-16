set(DESKTOP_APP_GUI_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/SettingsDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/AutoResizeStackedWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/BalloonToolTip.h
    ${CMAKE_CURRENT_LIST_DIR}/BlurredShadowEffect.h
    ${CMAKE_CURRENT_LIST_DIR}/ButtonIconManager.h
    ${CMAKE_CURRENT_LIST_DIR}/DateTimeFormatter.h
    ${CMAKE_CURRENT_LIST_DIR}/LowDiskSpaceDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/EventHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/InfoDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/MegaDelegateHoverManager.h
    ${CMAKE_CURRENT_LIST_DIR}/MegaNodeNames.h
    ${CMAKE_CURRENT_LIST_DIR}/NotificationsSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/OverQuotaDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/ScanningWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/QtPositioningBugFixer.h
    ${CMAKE_CURRENT_LIST_DIR}/PasswordLineEdit.h
    ${CMAKE_CURRENT_LIST_DIR}/UploadToMegaDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/PasteMegaLinksDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/ImportMegaLinksDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/ImportListWidgetItem.h
    ${CMAKE_CURRENT_LIST_DIR}/CrashReportDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/MultiQFileDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/MegaProxyStyle.h
    ${CMAKE_CURRENT_LIST_DIR}/AccountDetailsDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/DownloadFromMegaDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/ChangeLogDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/StreamingFromMegaDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/MegaProgressCustomDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/MegaInputDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/AvatarWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/MenuItemAction.h
    ${CMAKE_CURRENT_LIST_DIR}/StatusInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/PSAwidget.h
    ${CMAKE_CURRENT_LIST_DIR}/ElidedLabel.h
    ${CMAKE_CURRENT_LIST_DIR}/ChangePassword.h
    ${CMAKE_CURRENT_LIST_DIR}/Login2FA.h
    ${CMAKE_CURRENT_LIST_DIR}/QRWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/CircularUsageProgressBar.h
    ${CMAKE_CURRENT_LIST_DIR}/HighDpiResize.h
    ${CMAKE_CURRENT_LIST_DIR}/BugReportDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/ProgressIndicatorDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/VerifyLockMessage.h
    ${CMAKE_CURRENT_LIST_DIR}/ViewLoadingScene.h
    ${CMAKE_CURRENT_LIST_DIR}/ViewLoadingMessage.h
    ${CMAKE_CURRENT_LIST_DIR}/WaitingSpinnerWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/ProxySettings.h
    ${CMAKE_CURRENT_LIST_DIR}/BandwidthSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/SwitchButton.h
    ${CMAKE_CURRENT_LIST_DIR}/GuiUtilities.h
    ${CMAKE_CURRENT_LIST_DIR}/CancelConfirmWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/RemoteItemUi.h
    ${CMAKE_CURRENT_LIST_DIR}/WordWrapLabel.h
    ${CMAKE_CURRENT_LIST_DIR}/ThemeManager.h
    ${CMAKE_CURRENT_LIST_DIR}/AccountTypeWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/BannerWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/ApiImageLabel.h
    ${CMAKE_CURRENT_LIST_DIR}/TabSelector.h
    ${CMAKE_CURRENT_LIST_DIR}/SearchLineEdit.h
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog/NodeNameSetterDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog/NewFolderDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog/RenameNodeDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorDelegates.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorModel.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorModelSpecialised.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorModelItem.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/RestoreNodeManager.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorTreeView.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorTreeViewWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelector.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorLoadingDelegate.h
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorSpecializations.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlClipboard.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDialogWrapper.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlWidgetWrapper.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlInstancesManager.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlItem.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDialogManager.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlManager.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlTheme.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/ApiEnums.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/StandardIconProvider.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/ChooseFolder.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/ChooseFile.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDeviceName.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/AccountInfoData.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/WhatsNewWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/UpdatesList.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/WhatsNewController.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/UpdatesModel.h
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/Onboarding.h
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/PasswordStrengthChecker.h
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/GuestQmlDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/OnboardingQmlDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/GuestContent.h
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions/ExclusionRulesModel.h
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions/SyncExclusions.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenParserWidgetManager.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/IconTokenizer.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/ButtonTokensByType.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenizableButtons.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenizableItems.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/IconLabel.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenPropertyNames.h
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenPropertySetter.h
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesComponent.h
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupsController.h
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesModel.h
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesController.h
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidates.h
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesFolderSizeRequester.h
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions/AddExclusionRule.h
    ${CMAKE_CURRENT_LIST_DIR}/syncs/SyncsComponent.h
    ${CMAKE_CURRENT_LIST_DIR}/syncs/SyncsQmlDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/syncs/Syncs.h
    ${CMAKE_CURRENT_LIST_DIR}/syncs/SyncsData.h
    ${CMAKE_CURRENT_LIST_DIR}/surveys/SurveyWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/surveys/SurveyComponent.h
    ${CMAKE_CURRENT_LIST_DIR}/surveys/Surveys.h
    ${CMAKE_CURRENT_LIST_DIR}/surveys/SurveyController.h
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs/MessageDialogComponent.h
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs/MessageDialogData.h
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs/MessageDialogOpener.h
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellComponent.h
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellController.h
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellModel.h
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellPlans.h
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellQmlDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageCacheManager.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/AlertFilterType.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/AlertItem.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/FilterAlertWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/NotificationItem.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserAlert.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessage.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageDelegate.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageModel.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserNotification.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/NotificationExpirationTimer.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/DeviceCentre.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/DeviceModel.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/DeviceData.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/SyncModel.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/QmlSyncData.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/SyncStatus.h
)

set(DESKTOP_APP_GUI_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/SettingsDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BalloonToolTip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BlurredShadowEffect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ButtonIconManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LowDiskSpaceDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DateTimeFormatter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EventHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/InfoDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MegaDelegateHoverManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverQuotaDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ScanningWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NotificationsSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/QtPositioningBugFixer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PasswordLineEdit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UploadToMegaDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PasteMegaLinksDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ImportMegaLinksDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ImportListWidgetItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CrashReportDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MultiQFileDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MegaProxyStyle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AccountDetailsDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DownloadFromMegaDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ChangeLogDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StreamingFromMegaDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MegaProgressCustomDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MegaInputDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AvatarWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MenuItemAction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StatusInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ChangePassword.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PSAwidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ElidedLabel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Login2FA.cpp
    ${CMAKE_CURRENT_LIST_DIR}/QRWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CircularUsageProgressBar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BugReportDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgressIndicatorDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VerifyLockMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ViewLoadingScene.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ViewLoadingMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WaitingSpinnerWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProxySettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BandwidthSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SwitchButton.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GuiUtilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CancelConfirmWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RemoteItemUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WordWrapLabel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ThemeManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AccountTypeWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BannerWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ApiImageLabel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TabSelector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SearchLineEdit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog/NodeNameSetterDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog/NewFolderDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog/RenameNodeDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorDelegates.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorProxyModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorModelSpecialised.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/NodeSelectorModelItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model/RestoreNodeManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorTreeView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorTreeViewWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorLoadingDelegate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/NodeSelectorSpecializations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlClipboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDialogWrapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlWidgetWrapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlInstancesManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDialogManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlTheme.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/StandardIconProvider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/ChooseFolder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/ChooseFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlDeviceName.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/AccountInfoData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/WhatsNewWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/WhatsNewController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/UpdatesModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qml/QmlUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/Onboarding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/PasswordStrengthChecker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/GuestQmlDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/OnboardingQmlDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/onboarding/GuestContent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions/ExclusionRulesModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions/SyncExclusions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenParserWidgetManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/IconTokenizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/ButtonTokensByType.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenizableButtons.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenizableItems.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/TokenPropertySetter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/TokenizableItems/IconLabel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesComponent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupsController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidates.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backups/BackupCandidatesFolderSizeRequester.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions/AddExclusionRule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/syncs/SyncsComponent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/syncs/SyncsQmlDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/syncs/Syncs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/syncs/SyncsData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surveys/SurveyWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surveys/SurveyComponent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surveys/Surveys.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surveys/SurveyController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs/MessageDialogComponent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs/MessageDialogData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs/MessageDialogOpener.cpp
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellComponent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellPlans.cpp
    ${CMAKE_CURRENT_LIST_DIR}/upsell/UpsellQmlDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageCacheManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/AlertFilterType.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/AlertItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/FilterAlertWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/NotificationItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserAlert.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageDelegate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserMessageProxyModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/UserNotification.cpp
    ${CMAKE_CURRENT_LIST_DIR}/user_messages/NotificationExpirationTimer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/DeviceCentre.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/DeviceModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/SyncModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre/QmlSyncData.cpp
)

# UI files additions
target_sources_conditional(${ExecutableTarget}
    FLAG WIN32
    QT_AWARE
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/win/UploadToMegaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/MegaProgressCustomDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/PSAwidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/Login2FA.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/AlertItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/FilterAlertWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/AlertFilterType.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/LockedPopOver.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/VerifyLockMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/OverQuotaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/ScanningWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/CancelConfirmWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/NodeNameSetterDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/ViewLoadingScene.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/ViewLoadingMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/NotificationItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/win/AccountTypeWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/win/NodeSelectorTreeViewWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/win/NodeSelectorLoadingDelegate.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/win/NodeSelector.ui
)

target_sources_conditional(${ExecutableTarget}
   FLAG APPLE
   QT_AWARE
   PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/macx/UploadToMegaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/MegaProgressCustomDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/PSAwidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/Login2FA.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/AlertItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/FilterAlertWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/AlertFilterType.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/VerifyLockMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/OverQuotaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/ScanningWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/CancelConfirmWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/NodeNameSetterDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/ViewLoadingScene.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/ViewLoadingMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/NotificationItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/macx/NodeSelectorTreeViewWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/macx/NodeSelectorLoadingDelegate.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/macx/NodeSelector.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/LockedPopOver.ui
    ${CMAKE_CURRENT_LIST_DIR}/macx/AccountTypeWidget.ui
)

target_sources_conditional(${ExecutableTarget}
    FLAG UNIX AND NOT APPLE
    QT_AWARE
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/linux/UploadToMegaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/MegaProgressCustomDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/PSAwidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/Login2FA.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/AlertItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/FilterAlertWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/AlertFilterType.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/LockedPopOver.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/VerifyLockMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/OverQuotaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/CancelConfirmWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/ScanningWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/NodeNameSetterDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/ViewLoadingScene.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/ViewLoadingMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/NotificationItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/linux/AccountTypeWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/linux/NodeSelectorTreeViewWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/linux/NodeSelectorLoadingDelegate.ui
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/linux/NodeSelector.ui
)

# Resources and platform-specific additions
target_sources_conditional(${ExecutableTarget}
    FLAG NOT APPLE
    QT_AWARE
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/LockedPopOver.h
    ${CMAKE_CURRENT_LIST_DIR}/LockedPopOver.cpp
)

target_sources_conditional(${ExecutableTarget}
   FLAG APPLE
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/macx/LockedPopOver.ui
)

target_sources_conditional(${ExecutableTarget}
   FLAG UNIX
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/PermissionsDialog.cpp
   ${CMAKE_CURRENT_LIST_DIR}/PermissionsWidget.cpp
   ${CMAKE_CURRENT_LIST_DIR}/PermissionsDialog.h
   ${CMAKE_CURRENT_LIST_DIR}/PermissionsWidget.h
)


# Not using expression generator due to autouic not able to resolve them causing errors
if (WIN32)
    set_property(TARGET ${ExecutableTarget}
        PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/win ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/win ${CMAKE_CURRENT_LIST_DIR}/ui
    )
elseif (APPLE)
    set_property(TARGET ${ExecutableTarget}
        PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/macx ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/macx ${CMAKE_CURRENT_LIST_DIR}/ui
    )
    elseif(UNIX)
        set_property(TARGET ${ExecutableTarget}
            PROPERTY AUTOUIC_SEARCH_PATHS
            ${CMAKE_CURRENT_LIST_DIR}/linux ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui/linux ${CMAKE_CURRENT_LIST_DIR}/ui
        )
endif()


set (DESKTOP_APP_TS_FILES
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_ar.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_de.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_en.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_es.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_fr.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_id.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_it.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_ja.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_ko.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_nl.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_pl.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_pt.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_ro.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_ru.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_th.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_tr.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_vi.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_zh_CN.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/MEGASyncStrings_zh_TW.ts
)

set_source_files_properties(${DESKTOP_APP_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/gui/translations)

if (NOT DontUseResources)

    # Load functions to process qrcs and create aliases
    include(desktopapp_resources_processing)

    process_resources_file("${CMAKE_CURRENT_LIST_DIR}/Resources_common.qrc")
    process_resources_file("${CMAKE_CURRENT_LIST_DIR}/Resources_qml.qrc")
    process_resources_file("${CMAKE_CURRENT_LIST_DIR}/Resources_light.qrc")
    process_resources_file("${CMAKE_CURRENT_LIST_DIR}/Resources_dark.qrc")

    qt5_add_binary_resources(Resources_common "${CMAKE_CURRENT_LIST_DIR}/Resources_common.qrc")
    qt5_add_binary_resources(Resources_qml "${CMAKE_CURRENT_LIST_DIR}/Resources_qml.qrc")
    qt5_add_binary_resources(Resources_light "${CMAKE_CURRENT_LIST_DIR}/Resources_light.qrc")
    qt5_add_binary_resources(Resources_dark "${CMAKE_CURRENT_LIST_DIR}/Resources_dark.qrc")
    qt5_add_binary_resources(qml "${CMAKE_CURRENT_LIST_DIR}/qml/qml.qrc")

    add_dependencies(MEGAsync Resources_common)
    add_dependencies(MEGAsync Resources_qml)
    add_dependencies(MEGAsync Resources_light)
    add_dependencies(MEGAsync Resources_dark)
    add_dependencies(MEGAsync qml)

if (NOT APPLE)
    add_custom_command(TARGET ${ExecutableTarget} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/Resources_common.rcc
                $<TARGET_FILE_DIR:MEGAsync>
        COMMENT "Copying Resources_common.rcc to target output directory"
    )

    add_custom_command(TARGET ${ExecutableTarget} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/Resources_qml.rcc
                $<TARGET_FILE_DIR:MEGAsync>
        COMMENT "Copying Resources_qml.rcc to target output directory"
    )

    add_custom_command(TARGET ${ExecutableTarget} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/Resources_light.rcc
                $<TARGET_FILE_DIR:MEGAsync>
        COMMENT "Copying Resources_light.rcc to target output directory"
    )


    add_custom_command(TARGET ${ExecutableTarget} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/Resources_dark.rcc
                $<TARGET_FILE_DIR:MEGAsync>
        COMMENT "Copying Resources_dark.rcc to target output directory"
    )

    add_custom_command(TARGET ${ExecutableTarget} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/qml.rcc
                $<TARGET_FILE_DIR:MEGAsync>
        COMMENT "Copying qml.rcc to target output directory"
    )
endif()

    qt5_add_translation(DESKTOP_APP_QM_FILES ${DESKTOP_APP_TS_FILES})
endif()

#Review to load only the resource file per platform and not all of them
set(DESKTOP_APP_GUI_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Resources_common.qrc
    ${CMAKE_CURRENT_LIST_DIR}/Resources_qml.qrc
    ${CMAKE_CURRENT_LIST_DIR}/Resources_light.qrc
    ${CMAKE_CURRENT_LIST_DIR}/Resources_dark.qrc
    ${CMAKE_CURRENT_LIST_DIR}/qml/qml.qrc
)

list(APPEND QML_IMPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gui/qml)
list(REMOVE_DUPLICATES QML_IMPORT_PATH)
set(QML_IMPORT_PATH ${QML_IMPORT_PATH} CACHE STRING "Qt Creator extra qml import paths" FORCE)

set (DESKTOP_APP_GUI_UI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/ui/AccountDetailsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/BugReportDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ChangePassword.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoveSyncConfirmationDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/BandwidthSettings.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/DownloadFromMegaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/PermissionsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/PermissionsWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ProxySettings.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/SettingsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/CrashReportDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/NotificationsSettings.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ProgressIndicatorDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoteItemUi.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/BannerWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/LowDiskSpaceDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/StreamingFromMegaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/MegaInputDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ChangeLogDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ImportListWidgetItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ImportMegaLinksDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/PasteMegaLinksDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/SearchLineEdit.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/TabSelector.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/SyncAccountFullMessage.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/InfoDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/StatusInfo.ui
)

target_sources_conditional(${ExecutableTarget}
   FLAG NOT DontUseResources
   QT_AWARE
   PRIVATE
   ${DESKTOP_APP_GUI_RESOURCES}
   ${DESKTOP_APP_QM_FILES}
)

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_GUI_HEADERS}
    ${DESKTOP_APP_GUI_SOURCES}
    ${DESKTOP_APP_GUI_UI_FILES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/NodeNameSetterDialog
    ${CMAKE_CURRENT_LIST_DIR}/node_selector
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/model
    ${CMAKE_CURRENT_LIST_DIR}/node_selector/gui
    ${CMAKE_CURRENT_LIST_DIR}/qml
    ${CMAKE_CURRENT_LIST_DIR}/onboarding
    ${CMAKE_CURRENT_LIST_DIR}/SyncExclusions
    ${CMAKE_CURRENT_LIST_DIR}/backups
    ${CMAKE_CURRENT_LIST_DIR}/upsell
    ${CMAKE_CURRENT_LIST_DIR}/surveys
    ${CMAKE_CURRENT_LIST_DIR}/message_dialogs
    ${CMAKE_CURRENT_LIST_DIR}/syncs
    ${CMAKE_CURRENT_LIST_DIR}/ui
    ${CMAKE_CURRENT_LIST_DIR}/user_messages
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer
)
target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})

if (UNIX AND NOT APPLE)

    # Install tray icons and resources for Linux

    # color
    set(HICOLOR "share/icons/hicolor/scalable/status")
    install(FILES gui/images/synching.svg RENAME megasynching.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${HICOLOR}"
    )
    install(FILES gui/images/warning.svg RENAME megawarning.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${HICOLOR}"
    )
    install(FILES gui/images/alert.svg RENAME megaalert.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${HICOLOR}"
    )
    install(FILES gui/images/paused.svg RENAME megapaused.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${HICOLOR}"
    )
    install(FILES gui/images/logging.svg RENAME megalogging.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${HICOLOR}"
    )
    install(FILES gui/images/uptodate.svg RENAME megauptodate.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${HICOLOR}"
    )

    # mono-dark
    set(MONOCOLOR "share/icons/ubuntu-mono-dark/status/24")
    install(FILES gui/images/synching_clear.svg RENAME megasynching.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${MONOCOLOR}"
    )
    install(FILES gui/images/warning_clear.svg RENAME megawarning.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${MONOCOLOR}"
    )
    install(FILES gui/images/alert_clear.svg RENAME megaalert.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${MONOCOLOR}"
    )
    install(FILES gui/images/paused_clear.svg RENAME megapaused.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${MONOCOLOR}"
    )
    install(FILES gui/images/logging_clear.svg RENAME megalogging.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${MONOCOLOR}"
    )
    install(FILES gui/images/uptodate_clear.svg RENAME megauptodate.svg
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../${MONOCOLOR}"
    )

    # rcc files
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Resources_common.rcc"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/megasync/resources"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Resources_light.rcc"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/megasync/resources"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Resources_dark.rcc"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/megasync/resources"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Resources_qml.rcc"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/megasync/resources"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qml.rcc"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/megasync/resources"
    )


endif()
