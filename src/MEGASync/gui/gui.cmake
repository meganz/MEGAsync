set(DESKTOP_APP_GUI_HEADERS
    gui/SettingsDialog.h
    gui/AutoResizeStackedWidget.h
    gui/BalloonToolTip.h
    gui/BlurredShadowEffect.h
    gui/ButtonIconManager.h
    gui/DateTimeFormatter.h
    gui/LowDiskSpaceDialog.h
    gui/EventHelper.h
    gui/InfoDialog.h
    gui/MegaDelegateHoverManager.h
    gui/MegaNodeNames.h
    gui/NotificationsSettings.h
    gui/OverQuotaDialog.h
    gui/ScanningWidget.h
    gui/QtPositioningBugFixer.h
    gui/PasswordLineEdit.h
    gui/UploadToMegaDialog.h
    gui/PasteMegaLinksDialog.h
    gui/ImportMegaLinksDialog.h
    gui/ImportListWidgetItem.h
    gui/CrashReportDialog.h
    gui/MultiQFileDialog.h
    gui/MegaProxyStyle.h
    gui/AccountDetailsDialog.h
    gui/DownloadFromMegaDialog.h
    gui/ChangeLogDialog.h
    gui/StreamingFromMegaDialog.h
    gui/MegaProgressCustomDialog.h
    gui/UpgradeDialog.h
    gui/PlanWidget.h
    gui/QMegaMessageBox.h
    gui/AvatarWidget.h
    gui/MenuItemAction.h
    gui/StatusInfo.h
    gui/PSAwidget.h
    gui/ElidedLabel.h
    gui/UpgradeOverStorage.h
    gui/ChangePassword.h
    gui/Login2FA.h
    gui/QRWidget.h
    gui/CircularUsageProgressBar.h
    gui/HighDpiResize.h
    gui/BugReportDialog.h
    gui/ProgressIndicatorDialog.h
    gui/VerifyLockMessage.h
    gui/ViewLoadingScene.h
    gui/MegaInfoMessage.h
    gui/WaitingSpinnerWidget.h
    gui/ProxySettings.h
    gui/BandwidthSettings.h
    gui/SwitchButton.h
    gui/GuiUtilities.h
    gui/CancelConfirmWidget.h
    gui/RemoteItemUi.h
    gui/WordWrapLabel.h
    gui/ThemeManager.h
    gui/AccountTypeWidget.h
    gui/NodeNameSetterDialog/NodeNameSetterDialog.h
    gui/NodeNameSetterDialog/NewFolderDialog.h
    gui/NodeNameSetterDialog/RenameNodeDialog.h
    gui/node_selector/model/NodeSelectorDelegates.h
    gui/node_selector/model/NodeSelectorProxyModel.h
    gui/node_selector/model/NodeSelectorModel.h
    gui/node_selector/model/NodeSelectorModelSpecialised.h
    gui/node_selector/model/NodeSelectorModelItem.h
    gui/node_selector/gui/NodeSelectorTreeView.h
    gui/node_selector/gui/NodeSelectorTreeViewWidget.h
    gui/node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.h
    gui/node_selector/gui/NodeSelector.h
    gui/node_selector/gui/NodeSelectorLoadingDelegate.h
    gui/node_selector/gui/SearchLineEdit.h
    gui/node_selector/gui/NodeSelectorSpecializations.h
    gui/qml/QmlClipboard.h
    gui/qml/QmlDialog.h
    gui/qml/QmlDialogWrapper.h
    gui/qml/QmlWidgetWrapper.h
    gui/qml/QmlInstancesManager.h
    gui/qml/QmlItem.h
    gui/qml/QmlDialogManager.h
    gui/qml/QmlManager.h
    gui/qml/QmlTheme.h
    gui/qml/ApiEnums.h
    gui/qml/StandardIconProvider.h
    gui/qml/ChooseFolder.h
    gui/qml/ChooseFile.h
    gui/qml/QmlDeviceName.h
    gui/qml/AccountInfoData.h
    gui/qml/WhatsNewWindow.h
    gui/qml/UpdatesList.h
    gui/qml/WhatsNewController.h
    gui/qml/UpdatesModel.h
    gui/qml/QmlUtils.h
    gui/onboarding/Onboarding.h
    gui/onboarding/PasswordStrengthChecker.h
    gui/onboarding/GuestQmlDialog.h
    gui/onboarding/OnboardingQmlDialog.h
    gui/onboarding/GuestContent.h
    gui/SyncExclusions/ExclusionRulesModel.h
    gui/SyncExclusions/SyncExclusions.h
    gui/tokenizer/TokenParserWidgetManager.h
    gui/tokenizer/IconTokenizer.h
    gui/backups/Backups.h
    gui/backups/BackupsController.h
    gui/backups/BackupsModel.h
    gui/SyncExclusions/AddExclusionRule.h
    gui/syncs/SyncsComponent.h
    gui/syncs/Syncs.h
    gui/user_messages/UserMessageCacheManager.h
    gui/user_messages/AlertFilterType.h
    gui/user_messages/AlertItem.h
    gui/user_messages/FilterAlertWidget.h
    gui/user_messages/NotificationItem.h
    gui/user_messages/UserAlert.h
    gui/user_messages/UserMessage.h
    gui/user_messages/UserMessageDelegate.h
    gui/user_messages/UserMessageModel.h
    gui/user_messages/UserMessageProxyModel.h
    gui/user_messages/UserNotification.h
    gui/user_messages/UserMessageWidget.h
    gui/user_messages/NotificationExpirationTimer.h
    gui/DeviceCentre/DeviceCentre.h
    gui/DeviceCentre/DeviceModel.h
    gui/DeviceCentre/DeviceData.h
    gui/DeviceCentre/SyncModel.h
    gui/DeviceCentre/QmlSyncData.h
    gui/DeviceCentre/SyncStatus.h
)

set(DESKTOP_APP_GUI_SOURCES
    gui/SettingsDialog.cpp
    gui/BalloonToolTip.cpp
    gui/BlurredShadowEffect.cpp
    gui/ButtonIconManager.cpp
    gui/LowDiskSpaceDialog.cpp
    gui/DateTimeFormatter.cpp
    gui/EventHelper.cpp
    gui/InfoDialog.cpp
    gui/MegaDelegateHoverManager.cpp
    gui/OverQuotaDialog.cpp
    gui/ScanningWidget.cpp
    gui/NotificationsSettings.cpp
    gui/QtPositioningBugFixer.cpp
    gui/PasswordLineEdit.cpp
    gui/UploadToMegaDialog.cpp
    gui/PasteMegaLinksDialog.cpp
    gui/ImportMegaLinksDialog.cpp
    gui/ImportListWidgetItem.cpp
    gui/CrashReportDialog.cpp
    gui/MultiQFileDialog.cpp
    gui/MegaProxyStyle.cpp
    gui/AccountDetailsDialog.cpp
    gui/DownloadFromMegaDialog.cpp
    gui/ChangeLogDialog.cpp
    gui/StreamingFromMegaDialog.cpp
    gui/MegaProgressCustomDialog.cpp
    gui/UpgradeDialog.cpp
    gui/PlanWidget.cpp
    gui/QMegaMessageBox.cpp
    gui/AvatarWidget.cpp
    gui/MenuItemAction.cpp
    gui/StatusInfo.cpp
    gui/ChangePassword.cpp
    gui/PSAwidget.cpp
    gui/ElidedLabel.cpp
    gui/UpgradeOverStorage.cpp
    gui/Login2FA.cpp
    gui/QRWidget.cpp
    gui/CircularUsageProgressBar.cpp
    gui/BugReportDialog.cpp
    gui/ProgressIndicatorDialog.cpp
    gui/VerifyLockMessage.cpp
    gui/MegaInfoMessage.cpp
    gui/ViewLoadingScene.cpp
    gui/WaitingSpinnerWidget.cpp
    gui/ProxySettings.cpp
    gui/BandwidthSettings.cpp
    gui/SwitchButton.cpp
    gui/GuiUtilities.cpp
    gui/CancelConfirmWidget.cpp
    gui/RemoteItemUi.cpp
    gui/WordWrapLabel.cpp
    gui/ThemeManager.cpp
    gui/AccountTypeWidget.cpp
    gui/NodeNameSetterDialog/NodeNameSetterDialog.cpp
    gui/NodeNameSetterDialog/NewFolderDialog.cpp
    gui/NodeNameSetterDialog/RenameNodeDialog.cpp
    gui/node_selector/model/NodeSelectorDelegates.cpp
    gui/node_selector/model/NodeSelectorProxyModel.cpp
    gui/node_selector/model/NodeSelectorModel.cpp
    gui/node_selector/model/NodeSelectorModelSpecialised.cpp
    gui/node_selector/model/NodeSelectorModelItem.cpp
    gui/node_selector/gui/NodeSelectorTreeView.cpp
    gui/node_selector/gui/NodeSelectorTreeViewWidget.cpp
    gui/node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.cpp
    gui/node_selector/gui/NodeSelector.cpp
    gui/node_selector/gui/NodeSelectorLoadingDelegate.cpp
    gui/node_selector/gui/SearchLineEdit.cpp
    gui/node_selector/gui/NodeSelectorSpecializations.cpp
    gui/qml/QmlClipboard.cpp
    gui/qml/QmlDialog.cpp
    gui/qml/QmlDialogWrapper.cpp
    gui/qml/QmlWidgetWrapper.cpp
    gui/qml/QmlInstancesManager.cpp
    gui/qml/QmlItem.cpp
    gui/qml/QmlDialogManager.cpp
    gui/qml/QmlManager.cpp
    gui/qml/QmlTheme.cpp
    gui/qml/StandardIconProvider.cpp
    gui/qml/ChooseFolder.cpp
    gui/qml/ChooseFile.cpp
    gui/qml/QmlDeviceName.cpp
    gui/qml/AccountInfoData.cpp
    gui/qml/WhatsNewWindow.cpp
    gui/qml/WhatsNewController.cpp
    gui/qml/UpdatesModel.cpp
    gui/qml/QmlUtils.cpp
    gui/onboarding/Onboarding.cpp
    gui/onboarding/PasswordStrengthChecker.cpp
    gui/onboarding/GuestQmlDialog.cpp
    gui/onboarding/OnboardingQmlDialog.cpp
    gui/onboarding/GuestContent.cpp
    gui/SyncExclusions/ExclusionRulesModel.cpp
    gui/SyncExclusions/SyncExclusions.cpp
    gui/tokenizer/TokenParserWidgetManager.cpp
    gui/tokenizer/IconTokenizer.cpp
    gui/backups/Backups.cpp
    gui/backups/BackupsController.cpp
    gui/backups/BackupsModel.cpp
    gui/SyncExclusions/AddExclusionRule.cpp
    gui/syncs/SyncsComponent.cpp
    gui/syncs/Syncs.cpp
    gui/user_messages/UserMessageCacheManager.cpp
    gui/user_messages/AlertFilterType.cpp
    gui/user_messages/AlertItem.cpp
    gui/user_messages/FilterAlertWidget.cpp
    gui/user_messages/NotificationItem.cpp
    gui/user_messages/UserAlert.cpp
    gui/user_messages/UserMessageDelegate.cpp
    gui/user_messages/UserMessageModel.cpp
    gui/user_messages/UserMessageProxyModel.cpp
    gui/user_messages/UserNotification.cpp
    gui/user_messages/NotificationExpirationTimer.cpp
    gui/DeviceCentre/DeviceCentre.cpp
    gui/DeviceCentre/DeviceModel.cpp
    gui/DeviceCentre/SyncModel.cpp
    gui/DeviceCentre/QmlSyncData.cpp
)

# UI files additions
target_sources_conditional(MEGAsync
    FLAG WIN32
    QT_AWARE
    PRIVATE
    gui/Resources_win.qrc
    gui/win/InfoDialog.ui
    gui/win/UploadToMegaDialog.ui
    gui/win/PasteMegaLinksDialog.ui
    gui/win/ImportMegaLinksDialog.ui
    gui/win/ImportListWidgetItem.ui
    gui/win/CrashReportDialog.ui
    gui/win/ChangeLogDialog.ui
    gui/win/StreamingFromMegaDialog.ui
    gui/win/MegaProgressCustomDialog.ui
    gui/win/PlanWidget.ui
    gui/win/UpgradeDialog.ui
    gui/win/StatusInfo.ui
    gui/win/PSAwidget.ui
    gui/win/UpgradeOverStorage.ui
    gui/win/Login2FA.ui
    gui/win/AlertItem.ui
    gui/win/FilterAlertWidget.ui
    gui/win/AlertFilterType.ui
    gui/win/LockedPopOver.ui
    gui/win/VerifyLockMessage.ui
    gui/win/MegaInfoMessage.ui
    gui/win/OverQuotaDialog.ui
    gui/win/ScanningWidget.ui
    gui/win/CancelConfirmWidget.ui
    gui/win/NodeNameSetterDialog.ui
    gui/win/LowDiskSpaceDialog.ui
    gui/win/ViewLoadingScene.ui
    gui/win/NotificationItem.ui
    gui/win/AccountTypeWidget.ui
    gui/node_selector/gui/win/NodeSelectorTreeViewWidget.ui
    gui/node_selector/gui/win/NodeSelectorLoadingDelegate.ui
    gui/node_selector/gui/win/NodeSelector.ui
    gui/node_selector/gui/win/SearchLineEdit.ui
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   gui/Resources_macx.qrc
   gui/macx/InfoDialog.ui
   gui/macx/UploadToMegaDialog.ui
   gui/macx/PasteMegaLinksDialog.ui
   gui/macx/ImportMegaLinksDialog.ui
   gui/macx/ImportListWidgetItem.ui
   gui/macx/CrashReportDialog.ui
   gui/macx/ChangeLogDialog.ui
   gui/macx/StreamingFromMegaDialog.ui
   gui/macx/MegaProgressCustomDialog.ui
   gui/macx/PlanWidget.ui
   gui/macx/UpgradeDialog.ui
   gui/macx/StatusInfo.ui
   gui/macx/PSAwidget.ui
   gui/macx/UpgradeOverStorage.ui
   gui/macx/Login2FA.ui
   gui/macx/AlertItem.ui
   gui/macx/FilterAlertWidget.ui
   gui/macx/AlertFilterType.ui
   gui/macx/VerifyLockMessage.ui
   gui/macx/MegaInfoMessage.ui
   gui/macx/OverQuotaDialog.ui
   gui/macx/ScanningWidget.ui
   gui/macx/CancelConfirmWidget.ui
   gui/macx/NodeNameSetterDialog.ui
   gui/macx/LowDiskSpaceDialog.ui
   gui/macx/ViewLoadingScene.ui
   gui/macx/NotificationItem.ui
   gui/node_selector/gui/macx/NodeSelectorTreeViewWidget.ui
   gui/node_selector/gui/macx/NodeSelectorLoadingDelegate.ui
   gui/node_selector/gui/macx/NodeSelector.ui
   gui/node_selector/gui/macx/SearchLineEdit.ui
   gui/macx/LockedPopOver.ui
   gui/macx/AccountTypeWidget.ui
)

target_sources_conditional(MEGAsync
    FLAG UNIX AND NOT APPLE
    QT_AWARE
    PRIVATE
    gui/Resources_linux.qrc
    gui/linux/InfoDialog.ui
    gui/linux/UploadToMegaDialog.ui
    gui/linux/PasteMegaLinksDialog.ui
    gui/linux/ImportMegaLinksDialog.ui
    gui/linux/ImportListWidgetItem.ui
    gui/linux/CrashReportDialog.ui
    gui/linux/ChangeLogDialog.ui
    gui/linux/StreamingFromMegaDialog.ui
    gui/linux/MegaProgressCustomDialog.ui
    gui/linux/PlanWidget.ui
    gui/linux/UpgradeDialog.ui
    gui/linux/StatusInfo.ui
    gui/linux/PSAwidget.ui
    gui/linux/UpgradeOverStorage.ui
    gui/linux/Login2FA.ui
    gui/linux/AlertItem.ui
    gui/linux/FilterAlertWidget.ui
    gui/linux/AlertFilterType.ui
    gui/linux/LockedPopOver.ui
    gui/linux/VerifyLockMessage.ui
    gui/linux/MegaInfoMessage.ui
    gui/linux/OverQuotaDialog.ui
    gui/linux/CancelConfirmWidget.ui
    gui/linux/ScanningWidget.ui
    gui/linux/NodeNameSetterDialog.ui
    gui/linux/LowDiskSpaceDialog.ui
    gui/linux/ViewLoadingScene.ui
    gui/linux/NotificationItem.ui
    gui/linux/AccountTypeWidget.ui
    gui/node_selector/gui/linux/NodeSelectorTreeViewWidget.ui
    gui/node_selector/gui/linux/NodeSelectorLoadingDelegate.ui
    gui/node_selector/gui/linux/NodeSelector.ui
    gui/node_selector/gui/linux/SearchLineEdit.ui
)

# Resources and platform-specific additions
target_sources_conditional(MEGAsync
    FLAG NOT APPLE
    QT_AWARE
    PRIVATE
    gui/LockedPopOver.h
    gui/LockedPopOver.cpp
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   gui/CocoaHelpButton.mm
   gui/QMacSpinningProgressIndicator.mm
   gui/QSegmentedControl.mm
   gui/QMacSpinningProgressIndicator.h
   gui/CocoaHelpButton.h
   gui/QSegmentedControl.h
   gui/macx/LockedPopOver.ui
)

target_sources_conditional(MEGAsync
   FLAG UNIX
   QT_AWARE
   PRIVATE
   gui/PermissionsDialog.cpp
   gui/PermissionsWidget.cpp
   gui/PermissionsDialog.h
   gui/PermissionsWidget.h
)


# Not using expression generator due to autouic not able to resolve them causing errors
if (WIN32)
    set_property(TARGET MEGAsync
        PROPERTY AUTOUIC_SEARCH_PATHS
        gui/win gui/node_selector/gui/win gui/ui
    )
elseif (APPLE)
    set_property(TARGET MEGAsync
        PROPERTY AUTOUIC_SEARCH_PATHS
        gui/macx gui/node_selector/gui/macx gui/ui
    )
    elseif(UNIX)
        set_property(TARGET MEGAsync
            PROPERTY AUTOUIC_SEARCH_PATHS
            gui/linux gui/node_selector/gui/linux gui/ui
        )
endif()


set (DESKTOP_APP_TS_FILES
    gui/translations/MEGASyncStrings_ar.ts
    gui/translations/MEGASyncStrings_de.ts
    gui/translations/MEGASyncStrings_en.ts
    gui/translations/MEGASyncStrings_es.ts
    gui/translations/MEGASyncStrings_fr.ts
    gui/translations/MEGASyncStrings_id.ts
    gui/translations/MEGASyncStrings_it.ts
    gui/translations/MEGASyncStrings_ja.ts
    gui/translations/MEGASyncStrings_ko.ts
    gui/translations/MEGASyncStrings_nl.ts
    gui/translations/MEGASyncStrings_pl.ts
    gui/translations/MEGASyncStrings_pt.ts
    gui/translations/MEGASyncStrings_ro.ts
    gui/translations/MEGASyncStrings_ru.ts
    gui/translations/MEGASyncStrings_th.ts
    gui/translations/MEGASyncStrings_vi.ts
    gui/translations/MEGASyncStrings_zh_CN.ts
    gui/translations/MEGASyncStrings_zh_TW.ts
)

set_source_files_properties(${DESKTOP_APP_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/gui/translations)
qt5_add_translation(DESKTOP_APP_QM_FILES ${DESKTOP_APP_TS_FILES})

set(DESKTOP_APP_GUI_RESOURCES
    gui/Resources.qrc
    gui/Resources_qml.qrc
    gui/qml/qml.qrc
)

list(APPEND QML_IMPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gui/qml)
list(REMOVE_DUPLICATES QML_IMPORT_PATH)
set(QML_IMPORT_PATH ${QML_IMPORT_PATH} CACHE STRING "Qt Creator extra qml import paths" FORCE)

set (DESKTOP_APP_GUI_UI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/ui/AccountDetailsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoveBackupDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/BugReportDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ChangePassword.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoveSyncConfirmationDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/BandwidthSettings.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/DownloadFromMegaDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/PermissionsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/PermissionsWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ProxySettings.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/SettingsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/NotificationsSettings.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/OpenBackupsFolder.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/ProgressIndicatorDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/SyncSettingsUIBase.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoteItemUi.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/SyncStallModeSelector.ui
)

set (DESKTOP_APP_GUI_UI_FILES_ROOT
    ${CMAKE_CURRENT_LIST_DIR}/ui/SettingsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/AccountDetailsDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoveBackupDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/ui/RemoveSyncConfirmationDialog.ui
)

list(JOIN DESKTOP_APP_GUI_UI_FILES "|" DESKTOP_APP_GUI_UI_FILES_TEMP )
list(JOIN DESKTOP_APP_GUI_UI_FILES_ROOT "|" DESKTOP_APP_GUI_UI_FILES_ROOT_TEMP )

target_compile_definitions(MEGAsync PRIVATE "DESKTOP_APP_GUI_UI_FILES=\"${DESKTOP_APP_GUI_UI_FILES_TEMP}\"")
target_compile_definitions(MEGAsync PRIVATE "DESKTOP_APP_GUI_UI_FILES_ROOT=\"${DESKTOP_APP_GUI_UI_FILES_ROOT_TEMP}\"")

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_GUI_HEADERS}
    ${DESKTOP_APP_GUI_SOURCES}
    ${DESKTOP_APP_GUI_RESOURCES}
    ${DESKTOP_APP_QM_FILES}
    ${DESKTOP_APP_GUI_UI_FILES}
)

target_include_directories(MEGAsync PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
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
    ${CMAKE_CURRENT_LIST_DIR}/syncs
    ${CMAKE_CURRENT_LIST_DIR}/ui
    ${CMAKE_CURRENT_LIST_DIR}/user_messages
    ${CMAKE_CURRENT_LIST_DIR}/DeviceCentre
    ${CMAKE_CURRENT_LIST_DIR}/tokenizer/
)
target_include_directories(MEGAsync PRIVATE ${INCLUDE_DIRECTORIES})

if (UNIX AND NOT APPLE)

    # Install tray icons for Linux

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

endif()
