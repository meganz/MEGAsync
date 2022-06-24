cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

set(MEGA_PROJECT_NAME "MEGAsync-desktop-" CACHE STRING "Project name, SDK will declare and append 32/64")
set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "Verbose output")

#add a target to have all files within qtcreator projects view
FILE(GLOB_RECURSE ui_files "${RepoDir}/src/MEGASync/gui/*.h" "${RepoDir}/src/MEGASync/gui/*/*.ui" "${RepoDir}/src/MEGASync/gui/*.qrc")
add_custom_target(ui_assets SOURCES ${ui_files})

if(CMAKE_HOST_APPLE)

    # Minimum deployment target differs if we are building for intel or arm64 targets
    # CMAKE_SYSTEM_PROCESSOR and CMAKE_HOST_SYSTEM_PROCESSOR are only available after project()
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE HOST_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Setup CMAKE_OSX_DEPLOYMENT_TARGET before project()
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64" OR (NOT CMAKE_OSX_ARCHITECTURES AND HOST_ARCHITECTURE STREQUAL "arm64"))
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.1" CACHE STRING "Minimum OS X deployment version")
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
    endif()

    message(STATUS "Minimum OS X deployment version is set to ${CMAKE_OSX_DEPLOYMENT_TARGET}")

    unset(HOST_ARCHITECTURE)

endif()

PROJECT(${MEGA_PROJECT_NAME})

#Qt settings
if (CMAKE_HOST_WIN32)
    set (UiDir "win")
    set (QTCOMPONENS_REQUIRED_PLATFORM WinExtras)
    set (TARGET_LINK_LIBRARIES_PLATFORM Qt5::WinExtras)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -DWIN32)
elseif (CMAKE_HOST_APPLE)
    set (UiDir "macx")
    set (QTCOMPONENS_REQUIRED_PLATFORM MacExtras)
    set (TARGET_LINK_LIBRARIES_PLATFORM Qt5::MacExtras)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -D__APPLE__ -D__GNUC__=4 -D__APPLE_CC__)
else()
    set (UiDir "linux")
    set (QTCOMPONENS_REQUIRED_PLATFORM Svg X11Extras)
    set (TARGET_LINK_LIBRARIES_PLATFORM Qt5::Svg Qt5::X11Extras xcb)
endif()

set (MEGA_QT_REQUIRED_COMPONENTS Core Network Gui Widgets LinguistTools ${QTCOMPONENS_REQUIRED_PLATFORM})
set (MEGA_QT_LINK_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network ${TARGET_LINK_LIBRARIES_PLATFORM})

set (UNCHECKED_ITERATORS 0 CACHE STRING "")

set (USE_LIBUV 1 CACHE STRING "")
set (USE_MEGAAPI 1 CACHE STRING "")
set (USE_MEDIAINFO 1 CACHE STRING "")
set (USE_LIBRAW 1 CACHE STRING "")
set (USE_SODIUM 1 CACHE STRING "")
set (USE_FFMPEG 1 CACHE STRING "")
set (USE_PDFIUM 1 CACHE STRING "")
set (USE_FREEIMAGE 1 CACHE STRING "")
set (USE_QT 1 CACHE STRING "")
set (ENABLE_LOG_PERFORMANCE 1 CACHE STRING "")
set (NO_READLINE 1 CACHE STRING "")
set (USE_DRIVE_NOTIFICATIONS 1 CACHE STRING "")

set (MEGA_LINK_DYNAMIC_CRT 1)  # since we are linking with QT official DLLs

set(RepoDir "${CMAKE_CURRENT_LIST_DIR}/../.."  CACHE STRING "")
set(MEGAsyncDir "${RepoDir}/src/MEGASync")
set(MEGAupdaterDir "${RepoDir}/src/MEGAUpdater")
set(MEGAShellExtDir "${RepoDir}/src/MEGAShellExt")

get_filename_component(MEGAsyncDir ${MEGAsyncDir} REALPATH)

## Use prebuild 3rdparties
set (USE_PREBUILT_3RDPARTY 1 CACHE STRING "")
set(prebuilt_dir "${MEGAsyncDir}/mega/bindings/qt/3rdparty")
#specify where 3rd party libraries are available
set(Mega3rdPartyDir "${prebuilt_dir}"  CACHE STRING "")


## Use vcpkg 3rdparties
#set (USE_THIRDPARTY_FROM_VCPKG 1 CACHE STRING "")
#set(Mega3rdPartyDir "C:/path/to/vcpkg/parent/folder"  CACHE STRING "")


# this line points to the MEGA SDK repo that you want to build MEGAsync against
include(${MEGAsyncDir}/mega/contrib/cmake/CMakeLists.txt)

set(3RDPARTY_RUNTIME_PATH "PATH=%PATH%" "${Mega3rdPartyDir}/vcpkg/installed/${VCPKG_TRIPLET}/debug/bin;${QT_DIR}/bin")

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${MEGAsyncDir}/gui/${UiDir}" "${MEGAsyncDir}/transfers/gui/${UiDir}" "${MEGAsyncDir}/stalled_issues/gui/${UiDir}" "${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/${UiDir}")

#AUTOMOC
set(CMAKE_AUTOMOC ON)
if (QT_VERSION VERSION_GREATER 5.7.99)
    message(STATUS "Enabling automoc predefines and including config.h")
    set(AUTOMOC_COMPILER_PREDEFINES ON)
    set (CMAKE_AUTOMOC_MOC_OPTIONS ${CMAKE_AUTOMOC_MOC_OPTIONS} --include "${MEGAsyncDir}/mega/include/config.h" )
else()
    message(STATUS "Disabling automoc predefines and including config.h. Qt version = ${QT_VERSION}")
endif()

include_directories( "${MEGAsyncDir}/mega/bindings/qt" )
include_directories( "${MEGAsyncDir}/control" )
include_directories( "${MEGAsyncDir}/model" )
include_directories( "${MEGAsyncDir}/gui" )
include_directories( "${MEGAsyncDir}/stalled_issues/model" )
include_directories( "${MEGAsyncDir}/stalled_issues/gui" )
include_directories( "${MEGAsyncDir}/stalled_issues/gui/${UiDir}" )
include_directories( "${MEGAsyncDir}/platform" )
include_directories( "${MEGAsyncDir}/gui/${UiDir}" )
include_directories( "${MEGAsyncDir}/transfers/gui" )
include_directories( "${MEGAsyncDir}/transfers/gui/${UiDir}" )


set (TS_FILES
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_ar.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_de.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_en.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_es.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_fr.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_id.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_it.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_ja.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_ko.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_nl.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_pl.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_pt.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_ro.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_ru.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_th.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_vi.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_zh_CN.ts
    ${MEGAsyncDir}/gui/translations/MEGASyncStrings_zh_TW.ts )

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${MEGAsyncDir}/gui/translations/")
qt5_add_translation(QM_FILES ${TS_FILES})

set (FORMS
    ${MEGAsyncDir}/gui/${UiDir}/AlertItem.ui
    ${MEGAsyncDir}/gui/${UiDir}/AlertFilterType.ui
    ${MEGAsyncDir}/gui/${UiDir}/BugReportDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/FilterAlertWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/InfoDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/NodeSelector.ui
    ${MEGAsyncDir}/gui/${UiDir}/FolderBinder.ui
    ${MEGAsyncDir}/gui/${UiDir}/BindFolderDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/UploadToMegaDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/PasteMegaLinksDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ImportMegaLinksDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ImportListWidgetItem.ui
    ${MEGAsyncDir}/gui/${UiDir}/CrashReportDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/SetupWizard.ui
    ${MEGAsyncDir}/gui/${UiDir}/SettingsDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/NotificationsSettings.ui
    ${MEGAsyncDir}/gui/${UiDir}/AccountDetailsDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/DownloadFromMegaDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ChangeLogDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/GuestWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/StreamingFromMegaDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/MegaProgressCustomDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/PlanWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/UpgradeDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/InfoWizard.ui
    ${MEGAsyncDir}/gui/${UiDir}/MegaSpeedGraph.ui
    ${MEGAsyncDir}/gui/${UiDir}/AddExclusionDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/StatusInfo.ui
    ${MEGAsyncDir}/gui/${UiDir}/PSAwidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/QSyncItemWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/UpgradeOverStorage.ui
    ${MEGAsyncDir}/gui/${UiDir}/ChangePassword.ui
    ${MEGAsyncDir}/gui/${UiDir}/Login2FA.ui
# added per platform    ${MEGAsyncDir}/gui/${UiDir}/LockedPopOver.ui
    ${MEGAsyncDir}/gui/${UiDir}/VerifyLockMessage.ui
    ${MEGAsyncDir}/gui/${UiDir}/MegaInfoMessage.ui
    ${MEGAsyncDir}/gui/${UiDir}/OverQuotaDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ProxySettings.ui
    ${MEGAsyncDir}/gui/${UiDir}/BandwidthSettings.ui
    ${MEGAsyncDir}/gui/${UiDir}/FileFolderNameSetterDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ScanningWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/CancelConfirmWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/InfoDialogTransfersWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/InfoDialogTransferDelegateWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransferWidgetHeaderItem.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransferManager.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransferManagerDragBackDrop.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransfersWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransfersStatusWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransfersSummaryWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransferManagerDelegateWidget.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/TransferManagerLoadingItem.ui
    ${MEGAsyncDir}/transfers/gui/${UiDir}/SomeIssuesOccurredMessage.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssueChooseWidget.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssueFilePath.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssueHeader.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssueLoadingItem.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssuesDialog.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssueTab.ui
    ${MEGAsyncDir}/stalled_issues/gui/${UiDir}/StalledIssueActionTitle.ui
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/${UiDir}/LocalAndRemoteDifferentWidget.ui
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/${UiDir}/OtherSideMissingOrBlocked.ui
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/${UiDir}/LocalAndRemoteNameConflicts.ui
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/${UiDir}/NameConflict.ui

    )


if (CMAKE_HOST_WIN32)
    set (FORMS ${FORMS} )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (FORMS ${FORMS}
        ${MEGAsyncDir}/gui/${UiDir}/PermissionsDialog.ui
        ${MEGAsyncDir}/gui/${UiDir}/PermissionsWidget.ui
        )
else()
    set (FORMS ${FORMS}
        ${MEGAsyncDir}/gui/${UiDir}/PermissionsDialog.ui
        ${MEGAsyncDir}/gui/${UiDir}/PermissionsWidget.ui
        )
endif()

set (MOC_INPUT
    ${MEGAsyncDir}/MegaApplication.h
    ${MEGAsyncDir}/DesktopNotifications.h
    ${MEGAsyncDir}/TransferQuota.h
    ${MEGAsyncDir}/RemovedSharesNotificator.h
    ${MEGAsyncDir}/UserAlertTimedClustering.h
    ${MEGAsyncDir}/ScaleFactorManager.h
    ${MEGAsyncDir}/CommonMessages.h
    ${MEGAsyncDir}/ScanStageController.h
    ${MEGAsyncDir}/control/ConnectivityChecker.h
    ${MEGAsyncDir}/control/CrashHandler.h
    ${MEGAsyncDir}/control/EncryptedSettings.h
    ${MEGAsyncDir}/control/ExportProcessor.h
    ${MEGAsyncDir}/control/HTTPServer.h
    ${MEGAsyncDir}/control/LinkProcessor.h
    ${MEGAsyncDir}/control/MegaDownloader.h
    ${MEGAsyncDir}/control/MegaSyncLogger.h
    ${MEGAsyncDir}/control/MegaUploader.h
    ${MEGAsyncDir}/control/Preferences.h
    ${MEGAsyncDir}/control/TransferRemainingTime.h
    ${MEGAsyncDir}/control/UpdateTask.h
    ${MEGAsyncDir}/control/ThreadPool.h
    ${MEGAsyncDir}/control/MegaController.h
    ${MEGAsyncDir}/control/TransferBatch.h
    ${MEGAsyncDir}/control/UserAttributesManager.h

    ${MEGAsyncDir}/model/SyncSettings.h
    ${MEGAsyncDir}/model/Model.h

    ${MEGAsyncDir}/gui/AlertItem.h
    ${MEGAsyncDir}/gui/AlertFilterType.h
    ${MEGAsyncDir}/gui/BugReportDialog.h
    ${MEGAsyncDir}/gui/CircularUsageProgressBar.h
    ${MEGAsyncDir}/gui/FilterAlertWidget.h
    ${MEGAsyncDir}/gui/MegaAlertDelegate.h
    ${MEGAsyncDir}/gui/QAlertsModel.h
    ${MEGAsyncDir}/gui/QFilterAlertsModel.h
    ${MEGAsyncDir}/gui/AccountDetailsDialog.h
    ${MEGAsyncDir}/gui/AddExclusionDialog.h
    ${MEGAsyncDir}/gui/AvatarWidget.h
    ${MEGAsyncDir}/gui/BindFolderDialog.h
    ${MEGAsyncDir}/gui/ChangeLogDialog.h
    ${MEGAsyncDir}/gui/ChangePassword.h
    ${MEGAsyncDir}/gui/PasswordLineEdit.h
    ${MEGAsyncDir}/gui/MegaProgressCustomDialog.h
    ${MEGAsyncDir}/gui/CrashReportDialog.h
    ${MEGAsyncDir}/gui/DownloadFromMegaDialog.h
    ${MEGAsyncDir}/gui/ElidedLabel.h
    ${MEGAsyncDir}/gui/FolderBinder.h
    ${MEGAsyncDir}/gui/GuestWidget.h
    ${MEGAsyncDir}/gui/HighDpiResize.h
    ${MEGAsyncDir}/gui/ImportListWidgetItem.h
    ${MEGAsyncDir}/gui/ImportMegaLinksDialog.h
    ${MEGAsyncDir}/gui/BalloonToolTip.h
    ${MEGAsyncDir}/gui/InfoDialog.h
    ${MEGAsyncDir}/gui/QtPositioningBugFixer.h
    ${MEGAsyncDir}/gui/InfoWizard.h
    ${MEGAsyncDir}/gui/Login2FA.h
    ${MEGAsyncDir}/gui/MegaProxyStyle.h
    ${MEGAsyncDir}/gui/MegaSpeedGraph.h
    ${MEGAsyncDir}/gui/MultiQFileDialog.h
    ${MEGAsyncDir}/gui/NodeSelector.h
    ${MEGAsyncDir}/gui/PasteMegaLinksDialog.h
    ${MEGAsyncDir}/gui/MegaProgressCustomDialog.h
    ${MEGAsyncDir}/gui/PlanWidget.h
    ${MEGAsyncDir}/gui/PSAwidget.h
    ${MEGAsyncDir}/gui/QAlertsModel.h
    ${MEGAsyncDir}/gui/MegaItemModel.h
    ${MEGAsyncDir}/gui/QSyncItemWidget.h
    ${MEGAsyncDir}/gui/SettingsDialog.h
    ${MEGAsyncDir}/gui/SetupWizard.h
    ${MEGAsyncDir}/gui/NotificationsSettings.h
    ${MEGAsyncDir}/gui/StatusInfo.h
    ${MEGAsyncDir}/gui/MegaItemProxyModel.h
    ${MEGAsyncDir}/gui/MegaItemTreeView.h
    ${MEGAsyncDir}/gui/StreamingFromMegaDialog.h
    ${MEGAsyncDir}/gui/UpgradeDialog.h
    ${MEGAsyncDir}/gui/UpgradeOverStorage.h
    ${MEGAsyncDir}/gui/UploadToMegaDialog.h
    ${MEGAsyncDir}/gui/VerifyLockMessage.h
    ${MEGAsyncDir}/gui/MegaInfoMessage.h
    ${MEGAsyncDir}/gui/WaitingSpinnerWidget.h
    ${MEGAsyncDir}/gui/OverQuotaDialog.h
    ${MEGAsyncDir}/gui/ProxySettings.h
    ${MEGAsyncDir}/gui/SwitchButton.h
    ${MEGAsyncDir}/gui/BandwidthSettings.h
    ${MEGAsyncDir}/gui/MegaItemDelegates.h
    ${MEGAsyncDir}/gui/GuiUtilities.h
    ${MEGAsyncDir}/gui/EventHelper.h
    ${MEGAsyncDir}/gui/UserAttributesRequests.h
    ${MEGAsyncDir}/gui/AutoResizeStackedWidget.h
    ${MEGAsyncDir}/gui/ViewLoadingScene.h
    ${MEGAsyncDir}/gui/ScanningWidget.h
    ${MEGAsyncDir}/gui/BlurredShadowEffect.h
    ${MEGAsyncDir}/gui/ButtonIconManager.h
    ${MEGAsyncDir}/gui/DialogGeometryRetainer.h
    ${MEGAsyncDir}/gui/FileFolderNameSetterDialog.h
    ${MEGAsyncDir}/gui/CancelConfirmWidget.h	

    ${MEGAsyncDir}/platform/notificator.h
    
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaGlobalListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaRequestListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaTransferListener.h
    
    ${MEGAsyncDir}/transfers/model/TransfersManagerSortFilterProxyModel.h
    ${MEGAsyncDir}/transfers/model/TransfersSortFilterProxyBaseModel.h
    ${MEGAsyncDir}/transfers/model/TransfersModel.h
    ${MEGAsyncDir}/transfers/model/InfoDialogTransfersProxyModel.h
    ${MEGAsyncDir}/transfers/gui/TransferItem.h
    ${MEGAsyncDir}/transfers/gui/TransferManager.h
    ${MEGAsyncDir}/transfers/gui/TransfersWidget.h
    ${MEGAsyncDir}/transfers/gui/TransfersStatusWidget.h
    ${MEGAsyncDir}/transfers/gui/TransfersSummaryWidget.h
    ${MEGAsyncDir}/transfers/gui/InfoDialogTransfersWidget.h
    ${MEGAsyncDir}/transfers/gui/MegaTransferDelegate.h
    ${MEGAsyncDir}/transfers/gui/MegaTransferView.h
    ${MEGAsyncDir}/transfers/gui/TransferWidgetHeaderItem.h
    ${MEGAsyncDir}/transfers/gui/SomeIssuesOccurredMessage.h
    ${MEGAsyncDir}/transfers/gui/TransferScanCancelUi.h

    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueChooseWidget.h
	${MEGAsyncDir}/stalled_issues/gui/StalledIssueActionTitle.h
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueFilePath.h
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueHeader.h
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueLoadingItem.h
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssuesDialog.h
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueTab.h
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/NameConflict.h
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.h
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.h
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/OtherSideMissingOrBlocked.h
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/StalledIssuesCaseHeaders.h
)

if (NOT CMAKE_HOST_APPLE)
    set (MOC_INPUT ${MOC_INPUT}
        ${MEGAsyncDir}/gui/LockedPopOver.h
        ${MEGAsyncDir}/gui/SwitchButton.h
        )
endif (NOT CMAKE_HOST_APPLE)

if (CMAKE_HOST_WIN32)
    set (MOC_INPUT ${MOC_INPUT}
        ${MEGAsyncDir}/platform/win/WinShellDispatcherTask.h
        )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (MOC_INPUT ${MOC_INPUT}
        ${MEGAsyncDir}/gui/PermissionsDialog.h
        ${MEGAsyncDir}/gui/PermissionsWidget.h


        ${MEGAsyncDir}/platform/linux/LinuxPlatform.h
        ${MEGAsyncDir}/platform/linux/ExtServer.h
        ${MEGAsyncDir}/platform/linux/NotifyServer.h
        )
else()
    set (MOC_INPUT ${MOC_INPUT}

        ${MEGAsyncDir}/gui/CocoaHelpButton.h
        ${MEGAsyncDir}/gui/CocoaSwitchButton.h
        ${MEGAsyncDir}/gui/MegaSystemTrayIcon.h

        ${MEGAsyncDir}/gui/PermissionsDialog.h
        ${MEGAsyncDir}/gui/PermissionsWidget.h

        ${MEGAsyncDir}/gui/QMacSpinningProgressIndicator.h
        ${MEGAsyncDir}/gui/QSegmentedControl.h

        ${MEGAsyncDir}/platform/macx/MacXPlatform.h
        ${MEGAsyncDir}/platform/macx/MacXFunctions.h
        ${MEGAsyncDir}/platform/macx/macnotificationhandler.h
        ${MEGAsyncDir}/platform/macx/NotificationDelegate.h
        ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.h
        ${MEGAsyncDir}/platform/macx/MEGAService.h
        ${MEGAsyncDir}/platform/macx/ClientSide.h
        ${MEGAsyncDir}/platform/macx/ServerSide.h
        ${MEGAsyncDir}/platform/macx/MacXExtServer.h
        ${MEGAsyncDir}/platform/macx/MacXExtServerService.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServer.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServerPrivate.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocket.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocketPrivate.h
        ${MEGAsyncDir}/platform/macx/LockedPopOver.h
        ${MEGAsyncDir}/platform/macx/Protocol.h
        )
endif()

set (SRCS
    ${MEGAsyncDir}/MegaApplication.cpp
    ${MEGAsyncDir}/DesktopNotifications.cpp
    ${MEGAsyncDir}/TransferQuota.cpp
    ${MEGAsyncDir}/RemovedSharesNotificator.cpp
    ${MEGAsyncDir}/UserAlertTimedClustering.cpp
    ${MEGAsyncDir}/ScaleFactorManager.cpp
    ${MEGAsyncDir}/CommonMessages.cpp
    ${MEGAsyncDir}/ScanStageController.cpp

    ${FORMS}
    ${MOC_INPUT}
    ${MEGAsyncDir}/gui/Resources_${UiDir}.qrc

    ${MEGAsyncDir}/gui/AlertItem.cpp
    ${MEGAsyncDir}/gui/AlertFilterType.cpp
    ${MEGAsyncDir}/gui/BugReportDialog.cpp
    ${MEGAsyncDir}/gui/BlurredShadowEffect.cpp
    ${MEGAsyncDir}/gui/CircularUsageProgressBar.cpp
    ${MEGAsyncDir}/gui/FilterAlertWidget.cpp
    ${MEGAsyncDir}/gui/MegaAlertDelegate.cpp
    ${MEGAsyncDir}/gui/NotificationsSettings.cpp
    ${MEGAsyncDir}/gui/QAlertsModel.cpp
    ${MEGAsyncDir}/gui/QFilterAlertsModel.cpp
    ${MEGAsyncDir}/gui/SettingsDialog.cpp
    ${MEGAsyncDir}/gui/BalloonToolTip.cpp
    ${MEGAsyncDir}/gui/InfoDialog.cpp
    ${MEGAsyncDir}/gui/QtPositioningBugFixer.cpp
    ${MEGAsyncDir}/gui/SetupWizard.cpp
    ${MEGAsyncDir}/gui/NodeSelector.cpp
    ${MEGAsyncDir}/gui/FolderBinder.cpp
    ${MEGAsyncDir}/gui/BindFolderDialog.cpp
    ${MEGAsyncDir}/gui/UploadToMegaDialog.cpp
    ${MEGAsyncDir}/gui/PasteMegaLinksDialog.cpp
    ${MEGAsyncDir}/gui/ImportMegaLinksDialog.cpp
    ${MEGAsyncDir}/gui/ImportListWidgetItem.cpp
    ${MEGAsyncDir}/gui/CrashReportDialog.cpp
    ${MEGAsyncDir}/gui/MultiQFileDialog.cpp
    ${MEGAsyncDir}/gui/MegaProxyStyle.cpp
    ${MEGAsyncDir}/gui/AccountDetailsDialog.cpp
    ${MEGAsyncDir}/gui/DownloadFromMegaDialog.cpp
    ${MEGAsyncDir}/gui/MegaItemModel.cpp
    ${MEGAsyncDir}/gui/MegaItem.cpp
    ${MEGAsyncDir}/gui/ChangeLogDialog.cpp
    ${MEGAsyncDir}/gui/GuestWidget.cpp
    ${MEGAsyncDir}/gui/HighDpiResize.cpp
    ${MEGAsyncDir}/gui/StreamingFromMegaDialog.cpp
    ${MEGAsyncDir}/gui/MegaProgressCustomDialog.cpp
    ${MEGAsyncDir}/gui/UpgradeDialog.cpp
    ${MEGAsyncDir}/gui/PlanWidget.cpp
    ${MEGAsyncDir}/gui/InfoWizard.cpp
    ${MEGAsyncDir}/gui/QSyncItemWidget.cpp
    ${MEGAsyncDir}/gui/QMegaMessageBox.cpp
    ${MEGAsyncDir}/gui/MegaSpeedGraph.cpp
    ${MEGAsyncDir}/gui/AvatarWidget.cpp
    ${MEGAsyncDir}/gui/MenuItemAction.cpp
    ${MEGAsyncDir}/gui/AddExclusionDialog.cpp
    ${MEGAsyncDir}/gui/StatusInfo.cpp
    ${MEGAsyncDir}/gui/ChangePassword.cpp
    ${MEGAsyncDir}/gui/PasswordLineEdit.cpp
    ${MEGAsyncDir}/gui/PSAwidget.cpp
    ${MEGAsyncDir}/gui/ElidedLabel.cpp
    ${MEGAsyncDir}/gui/UpgradeOverStorage.cpp
    ${MEGAsyncDir}/gui/Login2FA.cpp
    ${MEGAsyncDir}/gui/VerifyLockMessage.cpp
    ${MEGAsyncDir}/gui/MegaInfoMessage.cpp
    ${MEGAsyncDir}/gui/WaitingSpinnerWidget.cpp
    ${MEGAsyncDir}/gui/OverQuotaDialog.cpp
    ${MEGAsyncDir}/gui/ProxySettings.cpp
    ${MEGAsyncDir}/gui/BandwidthSettings.cpp
    ${MEGAsyncDir}/gui/SwitchButton.cpp
    ${MEGAsyncDir}/gui/MegaItemDelegates.cpp
    ${MEGAsyncDir}/gui/MegaItemProxyModel.cpp
    ${MEGAsyncDir}/gui/MegaItemTreeView.cpp
    ${MEGAsyncDir}/gui/GuiUtilities.cpp
    ${MEGAsyncDir}/gui/UserAttributesRequests.cpp
    ${MEGAsyncDir}/gui/ButtonIconManager.cpp
    ${MEGAsyncDir}/gui/EventHelper.cpp
    ${MEGAsyncDir}/gui/GuiUtilities.cpp
    ${MEGAsyncDir}/gui/ScanningWidget.cpp
    ${MEGAsyncDir}/gui/BlurredShadowEffect.cpp
    ${MEGAsyncDir}/gui/CancelConfirmWidget.cpp
    ${MEGAsyncDir}/gui/MegaDelegateHoverManager.cpp
    ${MEGAsyncDir}/gui/FileFolderNameSetterDialog.cpp

    ${MEGAsyncDir}/transfers/gui/TransfersStatusWidget.cpp
    ${MEGAsyncDir}/transfers/gui/TransferItem.cpp
    ${MEGAsyncDir}/transfers/gui/InfoDialogTransfersWidget.cpp
    ${MEGAsyncDir}/transfers/gui/TransferManager.cpp
    ${MEGAsyncDir}/transfers/gui/TransfersWidget.cpp
    ${MEGAsyncDir}/transfers/gui/MegaTransferDelegate.cpp
    ${MEGAsyncDir}/transfers/gui/MegaTransferView.cpp
    ${MEGAsyncDir}/transfers/gui/TransfersStatusWidget.cpp
    ${MEGAsyncDir}/transfers/gui/TransfersSummaryWidget.cpp
    ${MEGAsyncDir}/transfers/gui/TransferWidgetHeaderItem.cpp
    ${MEGAsyncDir}/transfers/gui/TransferScanCancelUi.cpp
    ${MEGAsyncDir}/transfers/gui/TransferManagerLoadingItem.cpp
    ${MEGAsyncDir}/transfers/model/TransfersManagerSortFilterProxyModel.cpp
    ${MEGAsyncDir}/transfers/model/TransfersModel.cpp
    ${MEGAsyncDir}/transfers/model/InfoDialogTransfersProxyModel.cpp
    ${MEGAsyncDir}/transfers/gui/TransferBaseDelegateWidget.cpp
    ${MEGAsyncDir}/transfers/gui/InfoDialogTransferDelegateWidget.cpp
    ${MEGAsyncDir}/transfers/gui/TransferManagerDelegateWidget.cpp
    ${MEGAsyncDir}/transfers/gui/SomeIssuesOccurredMessage.cpp

    ${MEGAsyncDir}/stalled_issues/model/StalledIssue.cpp
    ${MEGAsyncDir}/stalled_issues/model/StalledIssuesModel.cpp
    ${MEGAsyncDir}/stalled_issues/model/StalledIssuesProxyModel.cpp
    ${MEGAsyncDir}/stalled_issues/model/StalledIssuesUtilities.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueChooseWidget.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueActionTitle.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueFilePath.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueHeader.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueLoadingItem.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssuesDialog.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueTab.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueBaseDelegateWidget.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssuesView.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssueDelegate.cpp
    ${MEGAsyncDir}/stalled_issues/gui/StalledIssuesDelegateWidgetsCache.cpp
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/NameConflict.cpp
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.cpp
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.cpp
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/OtherSideMissingOrBlocked.cpp
    ${MEGAsyncDir}/stalled_issues/gui/stalled_issues_cases/StalledIssuesCaseHeaders.cpp

    ${MEGAsyncDir}/mega/bindings/qt/QTMegaRequestListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaTransferListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaGlobalListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaEvent.cpp

    ${MEGAsyncDir}/control/HTTPServer.cpp
    ${MEGAsyncDir}/control/Preferences.cpp
    ${MEGAsyncDir}/control/LinkProcessor.cpp
    ${MEGAsyncDir}/control/MegaUploader.cpp
    ${MEGAsyncDir}/control/UpdateTask.cpp
    ${MEGAsyncDir}/control/ThreadPool.cpp
    ${MEGAsyncDir}/control/EncryptedSettings.cpp
    ${MEGAsyncDir}/control/CrashHandler.cpp
    ${MEGAsyncDir}/control/ExportProcessor.cpp
    ${MEGAsyncDir}/control/Utilities.cpp
    ${MEGAsyncDir}/control/MegaDownloader.cpp
    ${MEGAsyncDir}/control/MegaSyncLogger.cpp
    ${MEGAsyncDir}/control/ConnectivityChecker.cpp
    ${MEGAsyncDir}/control/TransferRemainingTime.cpp
    ${MEGAsyncDir}/control/MegaController.cpp
    ${MEGAsyncDir}/control/TransferBatch.cpp
    ${MEGAsyncDir}/control/UserAttributesManager.cpp

    ${MEGAsyncDir}/model/SyncSettings.cpp
    ${MEGAsyncDir}/model/Model.cpp

    ${MEGAsyncDir}/platform/notificator.cpp

    ${MEGAsyncDir}/qtlockedfile/qtlockedfile.cpp

)


if (NOT CMAKE_HOST_APPLE)
   set (SRCS ${SRCS}
        ${MEGAsyncDir}/gui/LockedPopOver.cpp
       )
endif (NOT CMAKE_HOST_APPLE)

if (CMAKE_HOST_WIN32)
    set (SRCS ${SRCS}

        ${MEGAsyncDir}/qtlockedfile/qtlockedfile_win.cpp


        ${MEGAsyncDir}/google_breakpad/client/windows/handler/exception_handler.cc
        ${MEGAsyncDir}/google_breakpad/common/windows/string_utils.cc
        ${MEGAsyncDir}/google_breakpad/common/windows/guid_string.cc
        ${MEGAsyncDir}/google_breakpad/client/windows/crash_generation/crash_generation_client.cc

        ${MEGAsyncDir}/control/win/Utilities.cpp

        ${MEGAsyncDir}/platform/win/WindowsPlatform.cpp
        ${MEGAsyncDir}/platform/win/WinShellDispatcherTask.cpp
        ${MEGAsyncDir}/platform/win/WinTrayReceiver.cpp
        ${MEGAsyncDir}/platform/win/wintoastlib.cpp

        ${MEGAsyncDir}/icon.rc
        )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (SRCS ${SRCS}

        ${MEGAsyncDir}/gui/PermissionsDialog.cpp
        ${MEGAsyncDir}/gui/PermissionsWidget.cpp

        ${MEGAsyncDir}/qtlockedfile/qtlockedfile_unix.cpp


        ${MEGAsyncDir}/google_breakpad/client/linux/crash_generation/crash_generation_client.cc
        ${MEGAsyncDir}/google_breakpad/client/linux/handler/exception_handler.cc
        ${MEGAsyncDir}/google_breakpad/client/linux/handler/minidump_descriptor.cc
        ${MEGAsyncDir}/google_breakpad/client/linux/minidump_writer/minidump_writer.cc
        ${MEGAsyncDir}/google_breakpad/client/linux/minidump_writer/linux_dumper.cc
        ${MEGAsyncDir}/google_breakpad/client/linux/minidump_writer/linux_ptrace_dumper.cc
        ${MEGAsyncDir}/google_breakpad/client/linux/log/log.cc
        ${MEGAsyncDir}/google_breakpad/client/minidump_file_writer.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/linux_libc_support.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/file_id.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/memory_mapped_file.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/safe_readlink.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/guid_creator.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/elfutils.cc
        ${MEGAsyncDir}/google_breakpad/common/string_conversion.cc
        ${MEGAsyncDir}/google_breakpad/common/convert_UTF.c

        ${MEGAsyncDir}/control/linux/Utilities.cpp

        ${MEGAsyncDir}/platform/linux/LinuxPlatform.cpp
        ${MEGAsyncDir}/platform/linux/ExtServer.cpp
        ${MEGAsyncDir}/platform/linux/NotifyServer.cpp
        )
else()
    set (SRCS ${SRCS}

        ${MEGAsyncDir}/gui/PermissionsDialog.cpp
        ${MEGAsyncDir}/gui/PermissionsWidget.cpp

        ${MEGAsyncDir}/gui/CocoaHelpButton.mm
        ${MEGAsyncDir}/gui/CocoaSwitchButton.mm
        ${MEGAsyncDir}/gui/MegaSystemTrayIcon.mm

        ${MEGAsyncDir}/gui/QMacSpinningProgressIndicator.mm
        ${MEGAsyncDir}/gui/QSegmentedControl.mm

        ${MEGAsyncDir}/qtlockedfile/qtlockedfile_unix.cpp

        ${MEGAsyncDir}/google_breakpad/client/mac/handler/exception_handler.cc
        ${MEGAsyncDir}/google_breakpad/client/mac/crash_generation/crash_generation_client.cc
        ${MEGAsyncDir}/google_breakpad/client/mac/crash_generation/crash_generation_server.cc
        ${MEGAsyncDir}/google_breakpad/client/mac/handler/minidump_generator.cc
        ${MEGAsyncDir}/google_breakpad/client/mac/handler/dynamic_images.cc
        ${MEGAsyncDir}/google_breakpad/client/mac/handler/breakpad_nlist_64.cc
        ${MEGAsyncDir}/google_breakpad/client/minidump_file_writer.cc
        ${MEGAsyncDir}/google_breakpad/common/mac/macho_id.cc
        ${MEGAsyncDir}/google_breakpad/common/mac/macho_walker.cc
        ${MEGAsyncDir}/google_breakpad/common/mac/macho_utilities.cc
        ${MEGAsyncDir}/google_breakpad/common/mac/string_utilities.cc
        ${MEGAsyncDir}/google_breakpad/common/mac/file_id.cc
        ${MEGAsyncDir}/google_breakpad/common/mac/bootstrap_compat.cc
        ${MEGAsyncDir}/google_breakpad/common/md5.cc
        ${MEGAsyncDir}/google_breakpad/common/string_conversion.cc
        ${MEGAsyncDir}/google_breakpad/common/linux/linux_libc_support.cc
        ${MEGAsyncDir}/google_breakpad/common/convert_UTF.c

        ${MEGAsyncDir}/google_breakpad/common/mac/MachIPC.mm

        ${MEGAsyncDir}/control/macx/Utilities.mm

        ${MEGAsyncDir}/platform/macx/MacXFunctions.h
        ${MEGAsyncDir}/platform/macx/macnotificationhandler.h
        ${MEGAsyncDir}/platform/macx/NotificationDelegate.h
        ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.h
        ${MEGAsyncDir}/platform/macx/MEGAService.h
        ${MEGAsyncDir}/platform/macx/ClientSide.h
        ${MEGAsyncDir}/platform/macx/ServerSide.h
        ${MEGAsyncDir}/platform/macx/MacXExtServer.h
        ${MEGAsyncDir}/platform/macx/MacXExtServerService.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServer.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServerPrivate.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocket.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocketPrivate.h
        ${MEGAsyncDir}/platform/macx/LockedPopOver.h
        ${MEGAsyncDir}/platform/macx/Protocol.h
        ${MEGAsyncDir}/platform/macx/QCustomMacToolbar.h

        ${MEGAsyncDir}/platform/macx/MacXPlatform.cpp
        ${MEGAsyncDir}/platform/macx/MacXFunctions.mm
        ${MEGAsyncDir}/platform/macx/macnotificationhandler.mm
        ${MEGAsyncDir}/platform/macx/NotificationDelegate.mm
        ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.mm
        ${MEGAsyncDir}/platform/macx/MEGAService.mm
        ${MEGAsyncDir}/platform/macx/ClientSide.mm
        ${MEGAsyncDir}/platform/macx/ServerSide.mm
        ${MEGAsyncDir}/platform/macx/MacXExtServer.mm
        ${MEGAsyncDir}/platform/macx/MacXExtServerService.cpp
        ${MEGAsyncDir}/platform/macx/MacXLocalServer.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalServerPrivate.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalSocket.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalSocketPrivate.mm
        ${MEGAsyncDir}/platform/macx/LockedPopOver.mm
        ${MEGAsyncDir}/platform/macx/QCustomMacToolbar.mm

        )
endif()


add_definitions( -DQT_DISABLE_DEPRECATED_BEFORE=0x000000 -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DUNICODE -DQT_WIDGETS_LIB )
add_definitions( -DSHOW_LOGS -D_CRT_SECURE_NO_WARNINGS -DMEGA_QT_LOGGING -DQT_NO_CAST_FROM_ASCII -DPSAPI_VERSION=1 -DQT_WINEXTRAS_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB )
 #TODO: remove UNICODE (in config.h) and use the WIN32 definition accordingly only for Windows

if (ENABLE_LOG_PERFORMANCE)
    add_definitions(-DENABLE_LOG_PERFORMANCE)
endif()

if(WIN32)
    add_definitions( -DUNICODE -D_UNICODE )  # needed for visual studio projects to use the unicode runtime libraries

    #supported windows version: 7 and beyond
    add_definitions( -DNTDDI_VERSION=NTDDI_WIN7 )
    add_definitions( -D_WIN32_WINNT=0x0601 ) # 0601: windows 7
endif()


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCREATE_COMPATIBLE_MINIDUMPS -DLOG_TO_STDOUT -DLOG_TO_LOGGER")

if(CMAKE_HOST_APPLE)
    set(MAC_RESOURCES "${MEGAsyncDir}/app.icns" "${MEGAsyncDir}/folder.icns" "${MEGAsyncDir}/folder_yosemite.icns" "${MEGAsyncDir}/appicon32.tiff")

    set_source_files_properties(${MAC_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(MEGAsync MACOSX_BUNDLE ${MAC_RESOURCES} ${MEGAsyncDir}/main.cpp ${SRCS} ${QM_FILES})
    set_target_properties(MEGAsync PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${MEGAsyncDir}/Info_MEGA.plist")
elseif(CMAKE_HOST_WIN32)
    add_executable(MEGAsync WIN32 ${MEGAsyncDir}/main.cpp ${SRCS} ${QM_FILES})
    set_property(TARGET MEGAsync PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set_target_properties(MEGAsync PROPERTIES VS_DEBUGGER_ENVIRONMENT "${3RDPARTY_RUNTIME_PATH}")
else()
    add_executable(MEGAsync ${MEGAsyncDir}/main.cpp ${SRCS} ${QM_FILES})
endif()


target_include_directories(MEGAsync PRIVATE
                                    ${MEGAsyncDir}
                                    ${MEGAsyncDir}/transfers
                                    ${MEGAsyncDir}/transfers/gui
                                    ${MEGAsyncDir}/transfers/model
                                    ${MEGAsyncDir}/google_breakpad
                                    )

if (CMAKE_HOST_WIN32)
    set(TARGET_LINK_LIBRARIES_PLATFORM ${TARGET_LINK_LIBRARIES_PLATFORM} ole32 Shell32 crypt32 taskschd Kernel32.lib Iphlpapi.lib Userenv.lib Psapi.lib )
    set_target_properties(MEGAsync  PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /LARGEADDRESSAWARE /SAFESEH:NO /DEBUG " )
    set_target_properties(MEGAsync  PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /LARGEADDRESSAWARE /SAFESEH:NO /DEBUG " )
else()

if (CMAKE_HOST_APPLE)
    set_source_files_properties(app.icns folder.icns folder_yosemite.icns appicon32.tiff PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

target_link_libraries(MEGAsync Mega Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network ${TARGET_LINK_LIBRARIES_PLATFORM})

# Amend manifest to tell Windows that the application is DPI aware (needed for Windows 8.1 and up)
IF (MSVC)
    ADD_CUSTOM_COMMAND(
        TARGET MEGAsync
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\MEGAsync.exe.manifest\" -inputresource:\"$<TARGET_FILE:MEGAsync>\"\;\#1 -outputresource:\"$<TARGET_FILE:MEGAsync>\"\;\#1
        COMMENT "Adding display aware manifest..."
    )
ENDIF(MSVC)

if (CMAKE_HOST_WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127")  # 4127: conditional expression is constant  (occurs in QT headers)
endif()

# Place empty.lproj and PkgInfo files, needed to translate strings in native dialogs according system locale and identify the bundle as an application within Finder.
if (CMAKE_HOST_APPLE)
    add_custom_command(
            TARGET MEGAsync
            POST_BUILD
            COMMAND xcrun actool --compile "$<TARGET_FILE_DIR:MEGAsync>/../Resources" --platform macosx --minimum-deployment-target ${CMAKE_OSX_DEPLOYMENT_TARGET} "${MEGAsyncDir}/gui/images/Images.xcassets" > /dev/null 2>&1
            COMMENT "Building Assets.car in Resources..."
            VERBATIM
            )
    add_custom_command(
            TARGET MEGAsync
            POST_BUILD
            COMMAND touch \"$<TARGET_FILE_DIR:MEGAsync>/../Resources/empty.lproj\"
            COMMENT "Adding empty.lproj file to Resources..."
            )
    add_custom_command(
            TARGET MEGAsync
            POST_BUILD
            COMMAND echo "APPL????" > \"$<TARGET_FILE_DIR:MEGAsync>/../PkgInfo\"
            COMMENT "Adding PkgInfo file to app bundle..."
            )

    execute_process(
        COMMAND bash "-c" "grep \"const QString Preferences::VERSION_STRING\" ${MEGAsyncDir}/control/Preferences.cpp | awk -F '\"' '{print $2}'"
        OUTPUT_VARIABLE MEGASYNC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    message("MEGAsync version = ${MEGASYNC_VERSION}")
    add_custom_command(
            TARGET MEGAsync
            POST_BUILD
            COMMAND /usr/libexec/PlistBuddy -c \"Set :CFBundleShortVersionString ${MEGASYNC_VERSION}\" \"$<TARGET_FILE_DIR:MEGAsync>/../Info.plist\"
            COMMENT "Setting version to Info.plist..."
            )
endif()

set(FULLREQUIREMENTS TRUE CACHE BOOL "Verbose output")

if(FULLREQUIREMENTS)
    if (USE_FFMPEG AND HAVE_FFMPEG)
        add_definitions(-DREQUIRE_HAVE_FFMPEG)
    endif()

    add_definitions(
        -DREQUIRE_HAVE_LIBUV
        -DREQUIRE_HAVE_LIBRAW
        -DREQUIRE_USE_MEDIAINFO

        #-DREQUIRE_ENABLE_CHAT
        #-DREQUIRE_ENABLE_BACKUPS
        #-DREQUIRE_ENABLE_WEBRTC
        #-DREQUIRE_ENABLE_EVT_TLS
        #-DREQUIRE_USE_PCRE
        )

    if (CMAKE_HOST_WIN32 OR CMAKE_HOST_APPLE OR build_64_bit)
        if (USE_PDFIUM)
            add_definitions(-DREQUIRE_HAVE_PDFIUM)
        endif()
    endif()

endif(FULLREQUIREMENTS)


#-------------- MEGA updater --------------------

set (UPDATER_FILES
    ${MEGAupdaterDir}/MegaUpdater.cpp
    ${MEGAupdaterDir}/UpdateTask.cpp
)

ImportStdVcpkgLibrary(cryptopp-staticcrt        cryptopp-staticcrt cryptopp-staticcrt libcryptopp libcryptopp)

if(CMAKE_HOST_APPLE)
    set (UPDATER_FILES
        ${UPDATER_FILES}
        ${MEGAupdaterDir}/MacUtils.mm
    )
    add_executable(MEGAupdater MACOSX_BUNDLE ${MAC_RESOURCES} ${UPDATER_FILES} )
    target_link_libraries(MEGAupdater cryptopp-staticcrt "-framework Cocoa -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Security")
    set_property(TARGET MEGAupdater PROPERTY AUTOMOC OFF)
elseif(CMAKE_HOST_WIN32)
    add_executable(MEGAupdater WIN32 ${UPDATER_FILES} )
    #add_executable(MEGAupdater ${UPDATER_FILES} )
    target_link_libraries(MEGAupdater cryptopp-staticcrt Urlmon.lib Shlwapi.lib)
    set_property(TARGET MEGAupdater PROPERTY AUTOMOC OFF)
    set_property(TARGET MEGAupdater PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_target_properties(MEGAupdater  PROPERTIES LINK_FLAGS_RELEASE " /DEBUG " )
    #set_target_properties(MEGAupdater  PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE  /ENTRY:WinMain ") #/LARGEADDRESSAWARE /SAFESEH:NO /DEBUG " )
endif()

#-------------- MEGA Shell Extension  --------------------

set (SHELLEXT_FILES
    ${MEGAShellExtDir}/ShellExt.cpp
    ${MEGAShellExtDir}/RegUtils.cpp
    ${MEGAShellExtDir}/dllmain.cpp
    ${MEGAShellExtDir}/ContextMenuExt.cpp
    ${MEGAShellExtDir}/ClassFactoryShellExtSyncing.cpp
    ${MEGAShellExtDir}/ClassFactoryShellExtSynced.cpp
    ${MEGAShellExtDir}/ClassFactoryShellExtPending.cpp
    ${MEGAShellExtDir}/ClassFactoryContextMenuExt.cpp
    ${MEGAShellExtDir}/ClassFactory.cpp
    ${MEGAShellExtDir}/MegaInterface.cpp
    ${MEGAShellExtDir}/MEGAShellExt.rc
    ${MEGAShellExtDir}/GlobalExportFunctions.def
)

if(CMAKE_HOST_WIN32)
    add_library(MEGAShellExt SHARED  ${SHELLEXT_FILES} )
    target_link_libraries(MEGAShellExt user32.lib ole32.lib oleaut32.lib gdi32.lib uuid.lib Advapi32.lib Shell32.lib)
    set_property(TARGET MEGAShellExt PROPERTY AUTOMOC OFF)
    set_property(TARGET MEGAShellExt PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_target_properties(MEGAShellExt  PROPERTIES LINK_FLAGS_RELEASE " /DEBUG " )
endif()

#-------------- MEGA Sync unit tests --------------------
add_library(catch INTERFACE)
target_include_directories(catch INTERFACE "${RepoDir}/tests/3rdparty/catch")
add_library(trompeloeil INTERFACE)
target_include_directories(catch INTERFACE "${RepoDir}/tests/3rdparty/trompeloeil")

set(MEGASyncUnitTestsDir "${RepoDir}/tests/MEGASyncUnitTests")
set(UNIT_TEST_FILES
    ${MEGASyncUnitTestsDir}/GuestWidgetTest.cpp
    ${MEGASyncUnitTestsDir}/control/TransferRemainingTime.Test.cpp
    ${MEGASyncUnitTestsDir}/Utilities.test.cpp
    ${MEGASyncUnitTestsDir}/ScaleFactorManager.Test.cpp
    ${MEGASyncUnitTestsDir}/main.cpp
    )
add_executable(MEGASync_unit_tests ${UNIT_TEST_FILES} ${SRCS} ${QM_FILES})

target_link_libraries(MEGASync_unit_tests
    catch
    trompeloeil
    Mega
    ${MEGA_QT_LINK_LIBRARIES}
    ${TARGET_LINK_LIBRARIES_PLATFORM}
    )

target_include_directories(MEGASync_unit_tests PRIVATE ${MEGAsyncDir} ${MEGAsyncDir}/google_breakpad )
