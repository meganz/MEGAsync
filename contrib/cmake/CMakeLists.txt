
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
if (WIN32)
    set (MEGA_LINK_DYNAMIC_CRT 1)  # since we are linking with QT official DLLs
else()
    set(USE_PTHREAD 1 CACHE STRING "")
endif()

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
set(CMAKE_AUTOUIC_SEARCH_PATHS "${MEGAsyncDir}/gui/${UiDir}"
                               "${MEGAsyncDir}/transfers/gui/${UiDir}"
							   "${MEGAsyncDir}/transfers/gui/DuplicatedNodeDialogs/${UiDir}")

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
include_directories( "${MEGAsyncDir}/platform" )
include_directories( "${MEGAsyncDir}/gui/${UiDir}" )
include_directories( "${MEGAsyncDir}/transfers/model" )
include_directories( "${MEGAsyncDir}/transfers/gui" )
include_directories( "${MEGAsyncDir}/transfers/gui/${UiDir}" )
include_directories( "${MEGAsyncDir}/transfers/gui/DuplicatedNodeDialogs" )
include_directories( "${MEGAsyncDir}/transfers/gui/DuplicatedNodeDialogs/${UiDir}" )


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
    ${MEGAsyncDir}/gui/${UiDir}/NodeNameSetterDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ScanningWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/CancelConfirmWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/BackupsWizard.ui
    ${MEGAsyncDir}/gui/${UiDir}/AddBackupDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/RemoveBackupDialog.ui
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
    ${MEGAsyncDir}/transfers/gui/DuplicatedNodeDialogs/${UiDir}/DuplicatedNodeDialog.ui
    ${MEGAsyncDir}/transfers/gui/DuplicatedNodeDialogs/${UiDir}/DuplicatedNodeItem.ui
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
    ${MEGAsyncDir}/EventUpdater.h
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
    ${MEGAsyncDir}/control/SyncController.h
    ${MEGAsyncDir}/control/UserAttributesManager.h
    ${MEGAsyncDir}/control/TextDecorator.h
    ${MEGAsyncDir}/model/SyncSettings.h
    ${MEGAsyncDir}/model/SyncModel.h
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
    ${MEGAsyncDir}/gui/BackupsWizard.h
    ${MEGAsyncDir}/gui/SyncsMenu.h
    ${MEGAsyncDir}/gui/AddBackupDialog.h
    ${MEGAsyncDir}/gui/RemoveBackupDialog.h
    ${MEGAsyncDir}/gui/GuiUtilities.h
    ${MEGAsyncDir}/gui/EventHelper.h
    ${MEGAsyncDir}/gui/AutoResizeStackedWidget.h
    ${MEGAsyncDir}/gui/UserAttributesRequests/FullName.h
    ${MEGAsyncDir}/gui/UserAttributesRequests/DeviceName.h
    ${MEGAsyncDir}/gui/UserAttributesRequests/Avatar.h
    ${MEGAsyncDir}/gui/BackupItemModel.h
    ${MEGAsyncDir}/gui/SyncItemModel.h
    ${MEGAsyncDir}/gui/SyncTableView.h
    ${MEGAsyncDir}/gui/BackupTableView.h
    ${MEGAsyncDir}/gui/ViewLoadingScene.h
    ${MEGAsyncDir}/gui/ScanningWidget.h
    ${MEGAsyncDir}/gui/BlurredShadowEffect.h
    ${MEGAsyncDir}/gui/ButtonIconManager.h
    ${MEGAsyncDir}/gui/DialogGeometryRetainer.h
    ${MEGAsyncDir}/gui/NodeNameSetterDialog/NodeNameSetterDialog.cpp
    ${MEGAsyncDir}/gui/NodeNameSetterDialog/NewFolderDialog.cpp
    ${MEGAsyncDir}/gui/NodeNameSetterDialog/RenameNodeDialog.cpp
    ${MEGAsyncDir}/gui/CancelConfirmWidget.h	

    ${MEGAsyncDir}/platform/notificator.h
	${MEGAsyncDir}/platform/PlatformStrings.h
    
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaGlobalListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaRequestListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaTransferListener.h
    
    ${MEGAsyncDir}/control/TransferBatch.h    
    