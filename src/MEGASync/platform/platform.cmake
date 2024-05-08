set(DESKTOP_APP_PLATFORM_HEADERS
    platform/Platform.h
    platform/AbstractPlatform.h
    platform/ShellNotifier.h
    platform/PowerOptions.h
    platform/PlatformStrings.h
)

set(DESKTOP_APP_PLATFORM_SOURCES
    platform/AbstractPlatform.cpp
    platform/Platform.cpp
    platform/ShellNotifier.cpp
)

target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   platform/win/PlatformImplementation.h
   platform/win/RecursiveShellNotifier.h
   platform/win/ThreadedQueueShellNotifier.h
   platform/win/WinShellDispatcherTask.h
   platform/win/WinTrayReceiver.h
   platform/win/wintoastlib.h
   platform/win/WintoastCompat.h
   platform/win/WinAPIShell.h
   platform/win/PlatformImplementation.cpp
   platform/win/RecursiveShellNotifier.cpp
   platform/win/ThreadedQueueShellNotifier.cpp
   platform/win/WinShellDispatcherTask.cpp
   platform/win/WinTrayReceiver.cpp
   platform/win/wintoastlib.cpp
   platform/win/PowerOptions.cpp
   platform/win/PlatformStrings.cpp
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   platform/macx/PlatformImplementation.h
   platform/macx/MacXFunctions.h
   platform/macx/MacXSystemServiceTask.h
   platform/macx/MEGAService.h
   platform/macx/ClientSide.h
   platform/macx/ServerSide.h
   platform/macx/MacXExtServer.h
   platform/macx/MacXLocalServer.h
   platform/macx/MacXLocalServerPrivate.h
   platform/macx/MacXLocalSocket.h
   platform/macx/MacXLocalSocketPrivate.h
   platform/macx/NSPopover+MISSINGBackgroundView.h
   platform/macx/Protocol.h
   platform/macx/MacXExtServerService.h
   platform/macx/QCustomMacToolbar.h
   platform/macx/NativeMacPopover.h
   platform/macx/NativeMacPopoverPrivate.h
   platform/macx/PlatformImplementation.cpp
   platform/macx/MacXExtServerService.cpp
   platform/macx/PlatformStrings.cpp
   platform/macx/MacXFunctions.mm
   platform/macx/MacXSystemServiceTask.mm
   platform/macx/MEGAService.mm
   platform/macx/ClientSide.mm
   platform/macx/ServerSide.mm
   platform/macx/MacXExtServer.mm
   platform/macx/MacXLocalServer.mm
   platform/macx/MacXLocalServerPrivate.mm
   platform/macx/MacXLocalSocket.mm
   platform/macx/MacXLocalSocketPrivate.mm
   platform/macx/NSPopover+MISSINGBackgroundView.mm
   platform/macx/QCustomMacToolbar.mm
   platform/macx/PowerOptions.mm
   platform/macx/NativeMacPopover.mm
   platform/macx/NativeMacPopoverPrivate.mm
   platform/macx/LockedPopOver.h
   platform/macx/LockedPopOver.mm
)

target_sources_conditional(MEGAsync
   FLAG UNIX AND NOT APPLE
   QT_AWARE
   PRIVATE
   platform/linux/PlatformImplementation.h
   platform/linux/ExtServer.h
   platform/linux/NotifyServer.h
   platform/linux/DolphinFileManager.h
   platform/linux/NautilusFileManager.h
   platform/linux/PlatformImplementation.cpp
   platform/linux/ExtServer.cpp
   platform/linux/NotifyServer.cpp
   platform/linux/PowerOptions.cpp
   platform/linux/PlatformStrings.cpp
   platform/linux/DolphinFileManager.cpp
   platform/linux/NautilusFileManager.cpp
)

if (WIN32)
    target_compile_definitions(MEGAsync
        PUBLIC
        _UNICODE
    )

    target_link_libraries(MEGAsync
        PRIVATE
        Shell32 Shlwapi Powrprof taskschd
    )
elseif (APPLE)
    target_link_libraries(MEGAsync
        PRIVATE
        "-framework IOKit"
    )
else ()
    find_package(Qt5 REQUIRED COMPONENTS X11Extras)
    target_link_libraries(MEGAsync
        PRIVATE
        Qt5::X11Extras
        xcb
    )
endif()

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_PLATFORM_HEADERS}
    ${DESKTOP_APP_PLATFORM_SOURCES}
)

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/platform)
