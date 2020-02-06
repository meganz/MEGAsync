cmake_minimum_required(VERSION 3.0)
set(MEGA_PROJECT_NAME "MEGAsync-desktop-" CACHE STRING "Project name, SDK will declare and append 32/64")
set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "Verbose output")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")

PROJECT(${MEGA_PROJECT_NAME})

set (UNCHECKED_ITERATORS 0 CACHE STRING "")

set (USE_LIBUV 1 CACHE STRING "")
set (USE_MEGAAPI 1 CACHE STRING "")
set (USE_MEDIAINFO 1 CACHE STRING "")
set (USE_LIBRAW 1 CACHE STRING "")
set (USE_SODIUM 1 CACHE STRING "")
set (USE_FFMPEG 1 CACHE STRING "")
set (HAVE_PDFIUM 1 CACHE STRING "")
set (USE_FREEIMAGE 0 CACHE STRING "") # thumbnail generation etc done with QT, so no need for freeimage
set (USE_QT 1)


set (USE_PREBUILT_3RDPARTY 1 CACHE STRING "")

set (MEGA_LINK_DYNAMIC_CRT 1)  # since we are linking with QT official DLLs

set(RepoDir "${CMAKE_CURRENT_LIST_DIR}/../.."  CACHE STRING "")
set(MEGAsyncDir "${RepoDir}/src/MEGASync")

set(prebuilt_dir "${MEGAsyncDir}/mega/bindings/qt/3rdparty")

#specify where 3rd party libraries are available
set(Mega3rdPartyDir "${prebuilt_dir}"  CACHE STRING "")


message("CONFIG: ${CONFIG}")

# this line points to the MEGA SDK repo that you want to build MEGAsync against
include(${MEGAsyncDir}/mega/contrib/cmake/CMakeLists.txt)


if (CMAKE_HOST_WIN32)
    if (build_64_bit)
        SET (QT_DIR "C:/Qt/Qt5.6.3_64/5.6.3/msvc2015_64" CACHE STRING "")
    else()
        SET (QT_DIR "C:/Qt/Qt5.6.3/5.6.3/msvc2015" CACHE STRING "")
    ENDIF()
    set (UIC_EXEC "${QT_DIR}/bin/uic.exe")
    set (RCC_EXEC "${QT_DIR}/bin/rcc.exe")
    set (MOC_EXEC "${QT_DIR}/bin/moc.exe")
else()
    if (CMAKE_HOST_APPLE)
        SET (QT_DIR "/QT/qt5/qtbase" CACHE STRING "")
        set(NATIVE_ICON "${MEGAsyncDir}/app.icns")
    ENDIF()
    set (UIC_EXEC "${QT_DIR}/bin/uic")
    set (RCC_EXEC "${QT_DIR}/bin/rcc")
    set (MOC_EXEC "${QT_DIR}/bin/moc")
ENDIF()

if (CMAKE_HOST_WIN32)
    set (UiDir "win")
elseif (CMAKE_HOST_APPLE)
    set (UiDir "macx")
else()
    set (UiDir "linux")
endif()

find_package(Qt5 5.6.3 COMPONENTS Core Network MacExtras Gui Widgets REQUIRED)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${MEGAsyncDir}/gui/${UiDir}")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -D__APPLE__ -D__GNUC__=4 -D__APPLE_CC__)

include_directories( "${QT_DIR}/include" )
include_directories( "${QT_DIR}/include/QtCore" )
include_directories( "${QT_DIR}/include/QtGui" )
include_directories( "${QT_DIR}/include/QtNetwork" )
include_directories( "${QT_DIR}/include/QtWidgets" )
include_directories( "${QT_DIR}/include/QtWinExtras" )
include_directories( "${QT_DIR}/include/QtMacExtras" )


if (NOT CMAKE_HOST_WIN32)
    include_directories( "${QT_DIR}/include/QtSvg" )
endif()

include_directories( "${MEGAsyncDir}/mega/bindings/qt" )
include_directories( "${MEGAsyncDir}/control" )
include_directories( "${MEGAsyncDir}/gui" )
include_directories( "${MEGAsyncDir}/platform" )
include_directories( "${MEGAsyncDir}/gui/${UiDir}" )

set (FORMS
    ${MEGAsyncDir}/gui/${UiDir}/AlertItem.ui
    ${MEGAsyncDir}/gui/${UiDir}/AlertFilterType.ui
    ${MEGAsyncDir}/gui/${UiDir}/BugReportDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/FilterAlertWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransfersStatusWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransfersSummaryWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/InfoDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/CustomTransferItem.ui
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
    ${MEGAsyncDir}/gui/${UiDir}/AccountDetailsDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/DownloadFromMegaDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/SizeLimitDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/MessageBox.ui
    ${MEGAsyncDir}/gui/${UiDir}/ChangeLogDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/GuestWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/StreamingFromMegaDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/MegaProgressCustomDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/ConfirmSSLexception.ui
    ${MEGAsyncDir}/gui/${UiDir}/PlanWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/UpgradeDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/InfoWizard.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransferManagerItem.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransferManager.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransfersWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransfersStateInfoWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/MegaSpeedGraph.ui
    ${MEGAsyncDir}/gui/${UiDir}/ActiveTransfersWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/AddExclusionDialog.ui
    ${MEGAsyncDir}/gui/${UiDir}/LocalCleanScheduler.ui
    ${MEGAsyncDir}/gui/${UiDir}/InfoDialogTransfersWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/StatusInfo.ui
    ${MEGAsyncDir}/gui/${UiDir}/PSAwidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/UpgradeOverStorage.ui
#    ${MEGAsyncDir}/gui/${UiDir}/UpgradeWidget.ui
    ${MEGAsyncDir}/gui/${UiDir}/ChangePassword.ui
    ${MEGAsyncDir}/gui/${UiDir}/Login2FA.ui
    ${MEGAsyncDir}/gui/${UiDir}/TransfersStatusWidget.ui )


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
    ${MEGAsyncDir}/control/UpdateTask.h
    ${MEGAsyncDir}/gui/AlertItem.h
    ${MEGAsyncDir}/gui/AlertFilterType.h
    ${MEGAsyncDir}/gui/BugReportDialog.h
    ${MEGAsyncDir}/gui/CircularUsageProgressBar.h
    ${MEGAsyncDir}/gui/FilterAlertWidget.h
    ${MEGAsyncDir}/gui/MegaAlertDelegate.h
    ${MEGAsyncDir}/gui/QAlertsModel.h
    ${MEGAsyncDir}/gui/QFilterAlertsModel.h
    ${MEGAsyncDir}/gui/TransfersStatusWidget.h
    ${MEGAsyncDir}/gui/TransfersSummaryWidget.h
    ${MEGAsyncDir}/gui/AccountDetailsDialog.h
    ${MEGAsyncDir}/gui/ActiveTransfersWidget.h
    ${MEGAsyncDir}/gui/AddExclusionDialog.h
    ${MEGAsyncDir}/gui/AvatarWidget.h
    ${MEGAsyncDir}/gui/BindFolderDialog.h
    ${MEGAsyncDir}/gui/ChangeLogDialog.h
    ${MEGAsyncDir}/gui/ChangePassword.h
    ${MEGAsyncDir}/gui/MegaProgressCustomDialog.h
    ${MEGAsyncDir}/gui/ConfirmSSLexception.h
    ${MEGAsyncDir}/gui/CrashReportDialog.h
    ${MEGAsyncDir}/gui/CustomTransferItem.h
    ${MEGAsyncDir}/gui/DownloadFromMegaDialog.h
    ${MEGAsyncDir}/gui/ElidedLabel.h
    ${MEGAsyncDir}/gui/FolderBinder.h
    ${MEGAsyncDir}/gui/GuestWidget.h
    ${MEGAsyncDir}/gui/HighDpiResize.h
    ${MEGAsyncDir}/gui/ImportListWidgetItem.h
    ${MEGAsyncDir}/gui/ImportMegaLinksDialog.h
    ${MEGAsyncDir}/gui/InfoDialog.h
    ${MEGAsyncDir}/gui/InfoDialogTransfersWidget.h
    ${MEGAsyncDir}/gui/InfoWizard.h
    ${MEGAsyncDir}/gui/LocalCleanScheduler.h
    ${MEGAsyncDir}/gui/TransfersStatusWidget.h
    ${MEGAsyncDir}/gui/Login2FA.h
    ${MEGAsyncDir}/gui/MegaProxyStyle.h
    ${MEGAsyncDir}/gui/MegaSpeedGraph.h
    ${MEGAsyncDir}/gui/MegaTransferDelegate.h
    ${MEGAsyncDir}/gui/MegaTransferView.h
    ${MEGAsyncDir}/gui/MessageBox.h
    ${MEGAsyncDir}/gui/MultiQFileDialog.h
    ${MEGAsyncDir}/gui/NodeSelector.h
    ${MEGAsyncDir}/gui/PasteMegaLinksDialog.h
    ${MEGAsyncDir}/gui/MegaProgressCustomDialog.h
    ${MEGAsyncDir}/gui/PlanWidget.h
    ${MEGAsyncDir}/gui/PSAwidget.h
    ${MEGAsyncDir}/gui/QActiveTransfersModel.h
    ${MEGAsyncDir}/gui/QCustomTransfersModel.h
    ${MEGAsyncDir}/gui/QFinishedTransfersModel.h
    ${MEGAsyncDir}/gui/QAlertsModel.h
    ${MEGAsyncDir}/gui/QMegaModel.h
    ${MEGAsyncDir}/gui/QTransfersModel.h
    ${MEGAsyncDir}/gui/SettingsDialog.h
    ${MEGAsyncDir}/gui/SetupWizard.h
    ${MEGAsyncDir}/gui/SizeLimitDialog.h
    ${MEGAsyncDir}/gui/StatusInfo.h
    ${MEGAsyncDir}/gui/StreamingFromMegaDialog.h
    ${MEGAsyncDir}/gui/TransferItem.h
    ${MEGAsyncDir}/gui/TransferManager.h
    ${MEGAsyncDir}/gui/TransferManagerItem.h
    ${MEGAsyncDir}/gui/TransfersStateInfoWidget.h
    ${MEGAsyncDir}/gui/TransfersWidget.h
    ${MEGAsyncDir}/gui/UpgradeDialog.h
    ${MEGAsyncDir}/gui/UpgradeOverStorage.h
#    ${MEGAsyncDir}/gui/UpgradeWidget.h
    ${MEGAsyncDir}/gui/UploadToMegaDialog.h
    ${MEGAsyncDir}/gui/UsageWidget.h
    ${MEGAsyncDir}/platform/notificator.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaGlobalListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaRequestListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaSyncListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaTransferListener.h
)

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
        ${MEGAsyncDir}/gui/MegaSystemTrayIcon.h

        ${MEGAsyncDir}/gui/PermissionsDialog.h
        ${MEGAsyncDir}/gui/PermissionsWidget.h

        ${MEGAsyncDir}/platform/macx/MacXPlatform.h
        ${MEGAsyncDir}/platform/macx/MacXFunctions.h
        ${MEGAsyncDir}/platform/macx/macnotificationhandler.h
        ${MEGAsyncDir}/platform/macx/NotificationDelegate.h
        ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.h
        ${MEGAsyncDir}/platform/macx/MEGAService.h
        ${MEGAsyncDir}/platform/macx/ClientSide.h
        ${MEGAsyncDir}/platform/macx/ServerSide.h
        ${MEGAsyncDir}/platform/macx/MacXExtServer.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServer.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServerPrivate.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocket.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocketPrivate.h
        ${MEGAsyncDir}/platform/macx/Protocol.h
        )
endif()

set (SRCS 
    ${MEGAsyncDir}/MegaApplication.cpp

    ${FORMS}
    ${MOC_INPUT}
    ${RCC_INPUT}

    ${MEGAsyncDir}/gui/AlertItem.cpp
    ${MEGAsyncDir}/gui/AlertFilterType.cpp
    ${MEGAsyncDir}/gui/BugReportDialog.cpp
    ${MEGAsyncDir}/gui/CircularUsageProgressBar.cpp
    ${MEGAsyncDir}/gui/FilterAlertWidget.cpp
    ${MEGAsyncDir}/gui/MegaAlertDelegate.cpp
    ${MEGAsyncDir}/gui/QAlertsModel.cpp
    ${MEGAsyncDir}/gui/QFilterAlertsModel.cpp
    ${MEGAsyncDir}/gui/TransfersStatusWidget.cpp
    ${MEGAsyncDir}/gui/TransfersSummaryWidget.cpp
    ${MEGAsyncDir}/gui/SettingsDialog.cpp
    ${MEGAsyncDir}/gui/InfoDialog.cpp
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
    ${MEGAsyncDir}/gui/SizeLimitDialog.cpp
    ${MEGAsyncDir}/gui/UsageWidget.cpp
    ${MEGAsyncDir}/gui/MessageBox.cpp
    ${MEGAsyncDir}/gui/QMegaModel.cpp
    ${MEGAsyncDir}/gui/MegaItem.cpp
    ${MEGAsyncDir}/gui/ChangeLogDialog.cpp
    ${MEGAsyncDir}/gui/GuestWidget.cpp
    ${MEGAsyncDir}/gui/HighDpiResize.cpp
    ${MEGAsyncDir}/gui/StreamingFromMegaDialog.cpp
    ${MEGAsyncDir}/gui/MegaProgressCustomDialog.cpp
    ${MEGAsyncDir}/gui/ConfirmSSLexception.cpp
    ${MEGAsyncDir}/gui/UpgradeDialog.cpp
    ${MEGAsyncDir}/gui/PlanWidget.cpp
    ${MEGAsyncDir}/gui/InfoWizard.cpp
    ${MEGAsyncDir}/gui/TransferManager.cpp
    ${MEGAsyncDir}/gui/TransfersWidget.cpp
    ${MEGAsyncDir}/gui/QTransfersModel.cpp
    ${MEGAsyncDir}/gui/QActiveTransfersModel.cpp
    ${MEGAsyncDir}/gui/QFinishedTransfersModel.cpp
    ${MEGAsyncDir}/gui/MegaTransferDelegate.cpp
    ${MEGAsyncDir}/gui/MegaTransferView.cpp
    ${MEGAsyncDir}/gui/QMegaMessageBox.cpp
    ${MEGAsyncDir}/gui/TransfersStateInfoWidget.cpp
    ${MEGAsyncDir}/gui/MegaSpeedGraph.cpp
    ${MEGAsyncDir}/gui/ActiveTransfersWidget.cpp
    ${MEGAsyncDir}/gui/AvatarWidget.cpp
    ${MEGAsyncDir}/gui/MenuItemAction.cpp
    ${MEGAsyncDir}/gui/AddExclusionDialog.cpp
    ${MEGAsyncDir}/gui/LocalCleanScheduler.cpp
    ${MEGAsyncDir}/gui/TransferManagerItem.cpp
    ${MEGAsyncDir}/gui/TransferItem.cpp
    ${MEGAsyncDir}/gui/InfoDialogTransfersWidget.cpp
    ${MEGAsyncDir}/gui/QCustomTransfersModel.cpp
    ${MEGAsyncDir}/gui/StatusInfo.cpp
    ${MEGAsyncDir}/gui/ChangePassword.cpp
    ${MEGAsyncDir}/gui/CustomTransferItem.cpp
    ${MEGAsyncDir}/gui/PSAwidget.cpp
    ${MEGAsyncDir}/gui/ElidedLabel.cpp
    ${MEGAsyncDir}/gui/UpgradeOverStorage.cpp
#    ${MEGAsyncDir}/gui/UpgradeWidget.cpp
    ${MEGAsyncDir}/gui/Login2FA.cpp
    ${MEGAsyncDir}/gui/TransfersStatusWidget.cpp

    ${MEGAsyncDir}/mega/bindings/qt/QTMegaRequestListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaTransferListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaGlobalListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaSyncListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaListener.cpp
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaEvent.cpp

    ${MEGAsyncDir}/control/HTTPServer.cpp
    ${MEGAsyncDir}/control/Preferences.cpp
    ${MEGAsyncDir}/control/LinkProcessor.cpp
    ${MEGAsyncDir}/control/MegaUploader.cpp
    ${MEGAsyncDir}/control/UpdateTask.cpp
    ${MEGAsyncDir}/control/EncryptedSettings.cpp
    ${MEGAsyncDir}/control/CrashHandler.cpp
    ${MEGAsyncDir}/control/ExportProcessor.cpp
    ${MEGAsyncDir}/control/Utilities.cpp
    ${MEGAsyncDir}/control/MegaDownloader.cpp
    ${MEGAsyncDir}/control/MegaSyncLogger.cpp
    ${MEGAsyncDir}/control/ConnectivityChecker.cpp

    ${MEGAsyncDir}/platform/notificator.cpp

    ${MEGAsyncDir}/qtlockedfile/qtlockedfile.cpp

)


if (CMAKE_HOST_WIN32)
    set (SRCS ${SRCS}

        ${MEGAsyncDir}/qtlockedfile/qtlockedfile_win.cpp


        ${MEGAsyncDir}/google_breakpad/client/windows/handler/exception_handler.cc
        ${MEGAsyncDir}/google_breakpad/common/windows/string_utils.cc
        ${MEGAsyncDir}/google_breakpad/common/windows/guid_string.cc
        ${MEGAsyncDir}/google_breakpad/client/windows/crash_generation/crash_generation_client.cc



        ${MEGAsyncDir}/platform/win/WindowsPlatform.cpp
        ${MEGAsyncDir}/platform/win/WinShellDispatcherTask.cpp
        ${MEGAsyncDir}/platform/win/WinTrayReceiver.cpp
        ${MEGAsyncDir}/platform/win/wintoastlib.cpp
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

        ${MEGAsyncDir}/platform/linux/LinuxPlatform.cpp
        ${MEGAsyncDir}/platform/linux/ExtServer.cpp
        ${MEGAsyncDir}/platform/linux/NotifyServer.cpp
        )
else()
    set (SRCS ${SRCS}

        ${MEGAsyncDir}/gui/PermissionsDialog.cpp
        ${MEGAsyncDir}/gui/PermissionsWidget.cpp

        ${MEGAsyncDir}/gui/CocoaHelpButton.mm
        ${MEGAsyncDir}/gui/MegaSystemTrayIcon.mm

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


        ${MEGAsyncDir}/platform/macx/MacXFunctions.h
        ${MEGAsyncDir}/platform/macx/macnotificationhandler.h
        ${MEGAsyncDir}/platform/macx/NotificationDelegate.h
        ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.h
        ${MEGAsyncDir}/platform/macx/MEGAService.h
        ${MEGAsyncDir}/platform/macx/ClientSide.h
        ${MEGAsyncDir}/platform/macx/ServerSide.h
        ${MEGAsyncDir}/platform/macx/MacXExtServer.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServer.h
        ${MEGAsyncDir}/platform/macx/MacXLocalServerPrivate.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocket.h
        ${MEGAsyncDir}/platform/macx/MacXLocalSocketPrivate.h
        ${MEGAsyncDir}/platform/macx/Protocol.h

        ${MEGAsyncDir}/platform/macx/MacXPlatform.cpp
        ${MEGAsyncDir}/platform/macx/MacXFunctions.mm
        ${MEGAsyncDir}/platform/macx/macnotificationhandler.mm
        ${MEGAsyncDir}/platform/macx/NotificationDelegate.mm
        ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.mm
        ${MEGAsyncDir}/platform/macx/MEGAService.mm
        ${MEGAsyncDir}/platform/macx/ClientSide.mm
        ${MEGAsyncDir}/platform/macx/ServerSide.mm
        ${MEGAsyncDir}/platform/macx/MacXExtServer.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalServer.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalServerPrivate.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalSocket.mm
        ${MEGAsyncDir}/platform/macx/MacXLocalSocketPrivate.mm

        ${MEGAsyncDir}/gui/Resources_macx.qrc
        )
endif()



add_definitions( -DQT_DISABLE_DEPRECATED_BEFORE=0x000000 -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DUNICODE -D_UNICODE -DNTDDI_VERSION=0x05010000 -D_WIN32_WINNT=0x0501 -DQT_WIDGETS_LIB )
add_definitions( -DSHOW_LOGS -D_CRT_SECURE_NO_WARNINGS -DMEGA_QT_LOGGING -DQT_NO_CAST_FROM_ASCII -DPSAPI_VERSION=1 -DQT_WINEXTRAS_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB )
add_definitions( -DENABLE_LOG_PERFORMANCE )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCREATE_COMPATIBLE_MINIDUMPS -DLOG_TO_STDOUT -DLOG_TO_LOGGER")

if(CMAKE_HOST_APPLE)
  set_source_files_properties(${NATIVE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable(MEGAsync MACOSX_BUNDLE ${NATIVE_ICON} ${SRCS})
  set_target_properties(MEGAsync PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${MEGAsyncDir}/Info_MEGA.plist")
elseif(CMAKE_HOST_WIN32)
    add_executable(MEGAsync WIN32 ${SRCS})
else()
    add_executable(MEGAsync ${SRCS})
endif()


target_include_directories(MEGAsync PRIVATE ${MEGAsyncDir} ${MEGAsyncDir}/google_breakpad )

if (CMAKE_HOST_WIN32)
target_link_libraries(MEGAsync Mega
            debug "${QT_DIR}/lib/Qt5Cored.lib" 
            debug "${QT_DIR}/lib/Qt5Guid.lib"
            debug "${QT_DIR}/lib/Qt5Networkd.lib" 
            debug "${QT_DIR}/lib/Qt5Widgetsd.lib" 
            debug "${QT_DIR}/lib/Qt5WinExtrasd.lib" 
            optimized "${QT_DIR}/lib/Qt5Core.lib" 
            optimized "${QT_DIR}/lib/Qt5Gui.lib"
            optimized "${QT_DIR}/lib/Qt5Network.lib" 
            optimized "${QT_DIR}/lib/Qt5Widgets.lib" 
            optimized "${QT_DIR}/lib/Qt5WinExtras.lib" 
            ole32 Shell32 crypt32 taskschd Kernel32.lib Iphlpapi.lib Userenv.lib Psapi.lib )

set_target_properties(MEGAsync  PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /LARGEADDRESSAWARE /SAFESEH:NO /DEBUG /DELAYLOAD:pdfium.dll" )
else()

    set_source_files_properties(app.icns folder.icns folder_yosemite.icns appicon32.tiff PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_link_libraries(MEGAsync Mega
    "-F${QT_DIR}/lib -framework QtCore -framework QtGui -framework QtMacExtras -framework QtNetwork -framework QtPrintSupport -framework QtWidgets")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

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
