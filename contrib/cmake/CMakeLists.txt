cmake_minimum_required(VERSION 3.0)
set(MEGA_PROJECT_NAME "MEGAsync-desktop-" CACHE STRING "Project name, SDK will declare and append 32/64")

PROJECT(${MEGA_PROJECT_NAME})

if (WIN32)
    if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
        SET(build_64_bit 1)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(build_64_bit 0)
    endif()
else()
    set (build_64_bit 1 CACHE STRING "")
endif()

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
set (QT_DIR "C:/Qt/Qt5.6.3/5.6.3/msvc2015" CACHE STRING "")


set (USE_PREBUILT_3RDPARTY 1 CACHE STRING "")

set(RepoDir "${CMAKE_CURRENT_LIST_DIR}/../.."  CACHE STRING "")
set(MEGAsyncDir "${RepoDir}/src/MEGASync")

set(prebuilt_dir "${MEGAsyncDir}/mega/third_party/3rdparty")

#specify where 3rd party libraries are available
set(Mega3rdPartyDir "${RepoDir}/../3rdParty_sdk"  CACHE STRING "")


message("CONFIG: ${CONFIG}")

# this line points to the MEGA SDK repo that you want to build MEGAsync against
include(${MEGAsyncDir}/mega/contrib/cmake/CMakeLists.txt)

set(AUTOMOC ON)

include_directories( "${QT_DIR}/include" )
include_directories( "${QT_DIR}/include/QtCore" )
include_directories( "${QT_DIR}/include/QtGui" )
include_directories( "${QT_DIR}/include/QtNetwork" )
include_directories( "${QT_DIR}/include/QtWidgets" )
include_directories( "${QT_DIR}/include/QtWinExtras" )

include_directories( "${MEGAsyncDir}/mega/bindings/qt" )
include_directories( "${MEGAsyncDir}/control" )
include_directories( "${MEGAsyncDir}/gui" )
include_directories( "${MEGAsyncDir}/gui/win" )

set (FORMS
    ${MEGAsyncDir}/gui/win/InfoDialog.ui
    ${MEGAsyncDir}/gui/win/CustomTransferItem.ui
    ${MEGAsyncDir}/gui/win/NodeSelector.ui
    ${MEGAsyncDir}/gui/win/FolderBinder.ui
    ${MEGAsyncDir}/gui/win/BindFolderDialog.ui
    ${MEGAsyncDir}/gui/win/UploadToMegaDialog.ui
    ${MEGAsyncDir}/gui/win/PasteMegaLinksDialog.ui
    ${MEGAsyncDir}/gui/win/ImportMegaLinksDialog.ui
    ${MEGAsyncDir}/gui/win/ImportListWidgetItem.ui
    ${MEGAsyncDir}/gui/win/CrashReportDialog.ui
    ${MEGAsyncDir}/gui/win/SetupWizard.ui
    ${MEGAsyncDir}/gui/win/SettingsDialog.ui
    ${MEGAsyncDir}/gui/win/AccountDetailsDialog.ui
    ${MEGAsyncDir}/gui/win/DownloadFromMegaDialog.ui
    ${MEGAsyncDir}/gui/win/SizeLimitDialog.ui
    ${MEGAsyncDir}/gui/win/MessageBox.ui
    ${MEGAsyncDir}/gui/win/ChangeLogDialog.ui
    ${MEGAsyncDir}/gui/win/GuestWidget.ui
    ${MEGAsyncDir}/gui/win/StreamingFromMegaDialog.ui
    ${MEGAsyncDir}/gui/win/ConfirmSSLexception.ui
    ${MEGAsyncDir}/gui/win/PlanWidget.ui
    ${MEGAsyncDir}/gui/win/UpgradeDialog.ui
    ${MEGAsyncDir}/gui/win/InfoWizard.ui
    ${MEGAsyncDir}/gui/win/TransferManagerItem.ui
    ${MEGAsyncDir}/gui/win/TransferManager.ui
    ${MEGAsyncDir}/gui/win/TransfersWidget.ui
    ${MEGAsyncDir}/gui/win/TransfersStateInfoWidget.ui
    ${MEGAsyncDir}/gui/win/MegaSpeedGraph.ui
    ${MEGAsyncDir}/gui/win/ActiveTransfersWidget.ui
    ${MEGAsyncDir}/gui/win/AddExclusionDialog.ui
    ${MEGAsyncDir}/gui/win/LocalCleanScheduler.ui
    ${MEGAsyncDir}/gui/win/InfoDialogTransfersWidget.ui
    ${MEGAsyncDir}/gui/win/StatusInfo.ui
    ${MEGAsyncDir}/gui/win/PSAwidget.ui
    ${MEGAsyncDir}/gui/win/UpgradeOverStorage.ui
    ${MEGAsyncDir}/gui/win/UpgradeWidget.ui
    ${MEGAsyncDir}/gui/win/ChangePassword.ui
    ${MEGAsyncDir}/gui/win/Login2FA.ui )

set(FORMS_HEADERS "")
foreach(F ${FORMS})
    get_filename_component(D ${F} PATH)
    get_filename_component(N ${F} NAME_WE)
    set (T ${D}/ui_${N}.h)
    add_custom_command(
        OUTPUT ${T}
        COMMAND "${QT_DIR}/bin/uic.exe" -o ${T} ${F}
        DEPENDS ${F}
        WORKING_DIRECTORY ${MEGAsyncDir}/gui/win
    )
    SET(FORMS_HEADERS ${FORMS_HEADERS} ${T} )
endforeach(F)
    
    
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
    ${MEGAsyncDir}/gui/AccountDetailsDialog.h
    ${MEGAsyncDir}/gui/ActiveTransfersWidget.h
    ${MEGAsyncDir}/gui/AddExclusionDialog.h
    ${MEGAsyncDir}/gui/AvatarWidget.h
    ${MEGAsyncDir}/gui/BindFolderDialog.h
    ${MEGAsyncDir}/gui/ChangeLogDialog.h
    ${MEGAsyncDir}/gui/ChangePassword.h
#    ${MEGAsyncDir}/gui/CocoaHelpButton.h
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
    ${MEGAsyncDir}/gui/Login2FA.h
    ${MEGAsyncDir}/gui/MegaProxyStyle.h
    ${MEGAsyncDir}/gui/MegaSpeedGraph.h
    ${MEGAsyncDir}/gui/MegaSystemTrayIcon.h
    ${MEGAsyncDir}/gui/MegaTransferDelegate.h
    ${MEGAsyncDir}/gui/MegaTransferView.h
    ${MEGAsyncDir}/gui/MessageBox.h
    ${MEGAsyncDir}/gui/MultiQFileDialog.h
    ${MEGAsyncDir}/gui/NodeSelector.h
    ${MEGAsyncDir}/gui/PasteMegaLinksDialog.h
#    ${MEGAsyncDir}/gui/PermissionsDialog.h
#    ${MEGAsyncDir}/gui/PermissionsWidget.h
    ${MEGAsyncDir}/gui/PlanWidget.h
    ${MEGAsyncDir}/gui/PSAwidget.h
    ${MEGAsyncDir}/gui/QActiveTransfersModel.h
    ${MEGAsyncDir}/gui/QCustomTransfersModel.h
    ${MEGAsyncDir}/gui/QFinishedTransfersModel.h
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
    ${MEGAsyncDir}/gui/UpgradeWidget.h
    ${MEGAsyncDir}/gui/UploadToMegaDialog.h
    ${MEGAsyncDir}/gui/UsageWidget.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaGlobalListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaRequestListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaSyncListener.h
    ${MEGAsyncDir}/mega/bindings/qt/QTMegaTransferListener.h
    ${MEGAsyncDir}/platform/notificator.h
#    ${MEGAsyncDir}/platform/linux/ExtServer.h
#    ${MEGAsyncDir}/platform/linux/NotifyServer.h
#    ${MEGAsyncDir}/platform/macx/macnotificationhandler.h
#    ${MEGAsyncDir}/platform/macx/MacXExtServer.h
#    ${MEGAsyncDir}/platform/macx/MacXLocalServer.h
#    ${MEGAsyncDir}/platform/macx/MacXLocalSocket.h
#    ${MEGAsyncDir}/platform/macx/MacXSystemServiceTask.h
    ${MEGAsyncDir}/platform/win/WinShellDispatcherTask.h  
)

set(MOC_OUTPUT "")
foreach(F ${MOC_INPUT})
    get_filename_component(D ${F} PATH)
    get_filename_component(N ${F} NAME_WE)
    set (T ${D}/moc_${N}.cpp)
    add_custom_command(
        OUTPUT ${T}
        COMMAND "${QT_DIR}/bin/moc.exe" -DWIN32 -DENABLE_SYNC -o ${T} ${F}
        DEPENDS ${F}
        WORKING_DIRECTORY ${MEGAsyncDir}/gui
    )
    SET(MOC_OUTPUT ${MOC_OUTPUT} ${T} )
endforeach(F)

set (RCC_INPUT ${MEGAsyncDir}/gui/Resources_win.qrc)
set(RCC_OUTPUT "")
foreach(F ${RCC_INPUT})
    get_filename_component(D ${F} PATH)
    get_filename_component(N ${F} NAME_WE)
    set (T ${D}/qrc_${N}.cpp)
    add_custom_command(
        OUTPUT ${T}
        COMMAND "${QT_DIR}/bin/rcc.exe" -o ${T} ${F}
        DEPENDS ${F}
        WORKING_DIRECTORY ${MEGAsyncDir}/gui
    )
    SET(RCC_OUTPUT ${RCC_OUTPUT} ${T} )
endforeach(F)

set (SRCS 
    ${MEGAsyncDir}/MegaApplication.cpp

    ${FORMS_HEADERS}
    ${MOC_OUTPUT}
    ${RCC_OUTPUT}
    
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
#    ${MEGAsyncDir}/gui/PermissionsDialog.cpp
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
    ${MEGAsyncDir}/gui/UpgradeWidget.cpp
    ${MEGAsyncDir}/gui/Login2FA.cpp
    
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
    ${MEGAsyncDir}/platform/win/WindowsPlatform.cpp
    ${MEGAsyncDir}/platform/win/WinShellDispatcherTask.cpp
    ${MEGAsyncDir}/platform/win/WinTrayReceiver.cpp
    ${MEGAsyncDir}/platform/win/wintoastlib.cpp

    ${MEGAsyncDir}/google_breakpad/client/windows/handler/exception_handler.cc
    ${MEGAsyncDir}/google_breakpad/common/windows/string_utils.cc
    ${MEGAsyncDir}/google_breakpad/common/windows/guid_string.cc
    ${MEGAsyncDir}/google_breakpad/client/windows/crash_generation/crash_generation_client.cc

    ${MEGAsyncDir}/qtlockedfile/qtlockedfile.cpp
    ${MEGAsyncDir}/qtlockedfile/qtlockedfile_win.cpp
)

add_definitions( -DQT_DISABLE_DEPRECATED_BEFORE=0x000000 -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DUNICODE -D_UNICODE -DNTDDI_VERSION=0x05010000 -D_WIN32_WINNT=0x0501 -DQT_WIDGETS_LIB )
add_definitions( -DSHOW_LOGS -D_CRT_SECURE_NO_WARNINGS -DMEGA_QT_LOGGING -DQT_NO_CAST_FROM_ASCII -DPSAPI_VERSION=1 -DQT_WINEXTRAS_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCREATE_COMPATIBLE_MINIDUMPS -DLOG_TO_STDOUT -DLOG_TO_LOGGER")

add_executable(MEGAsync ${SRCS})
target_include_directories(MEGAsync PRIVATE ${MEGAsyncDir} ${MEGAsyncDir}/google_breakpad )
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

#set_target_properties(MEGAsync  PROPERTIES LINK_FLAGS "/ENTRY:\"wWinMainCRTStartup\" /SUBSYSTEM:WINDOWS" )


# Amend manifest to tell Windows that the application is DPI aware (needed for Windows 8.1 and up)
IF (MSVC)
    ADD_CUSTOM_COMMAND(
        TARGET MEGAsync
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\MEGAsync.exe.manifest\" -inputresource:\"$<TARGET_FILE:MEGAsync>\"\;\#1 -outputresource:\"$<TARGET_FILE:MEGAsync>\"\;\#1
        COMMENT "Adding display aware manifest..." 
    )
ENDIF(MSVC)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127")  # 4127: conditional expression is constant  (occurs in QT headers)
endif()
