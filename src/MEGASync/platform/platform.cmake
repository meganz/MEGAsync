set(DESKTOP_APP_PLATFORM_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Platform.h
    ${CMAKE_CURRENT_LIST_DIR}/AbstractPlatform.h
    ${CMAKE_CURRENT_LIST_DIR}/ShellNotifier.h
    ${CMAKE_CURRENT_LIST_DIR}/PowerOptions.h
    ${CMAKE_CURRENT_LIST_DIR}/PlatformStrings.h
)

set(DESKTOP_APP_PLATFORM_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/AbstractPlatform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Platform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShellNotifier.cpp
)

if (WIN32)
    set(LAF_TOKEN $ENV{LAF_TOKEN})
    configure_file(${CMAKE_CURRENT_LIST_DIR}/win/Laf.h.in ${CMAKE_CURRENT_BINARY_DIR}/win/Laf.h @ONLY)
endif()

target_sources_conditional(${ExecutableTarget}
   FLAG WIN32
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/win/PlatformImplementation.h
   ${CMAKE_CURRENT_LIST_DIR}/win/RecursiveShellNotifier.h
   ${CMAKE_CURRENT_LIST_DIR}/win/ThreadedQueueShellNotifier.h
   ${CMAKE_CURRENT_LIST_DIR}/win/WinShellDispatcherTask.h
   ${CMAKE_CURRENT_LIST_DIR}/win/WinTrayReceiver.h
   ${CMAKE_CURRENT_LIST_DIR}/win/wintoastlib.h
   ${CMAKE_CURRENT_LIST_DIR}/win/WintoastCompat.h
   ${CMAKE_CURRENT_LIST_DIR}/win/WinAPIShell.h
   ${CMAKE_CURRENT_LIST_DIR}/win/DesktopManager.h
   ${CMAKE_CURRENT_LIST_DIR}/win/PlatformImplementation.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/RecursiveShellNotifier.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/ThreadedQueueShellNotifier.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/WinShellDispatcherTask.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/WinTrayReceiver.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/wintoastlib.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/PowerOptions.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/PlatformStrings.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/DesktopManager.cpp
   ${CMAKE_CURRENT_LIST_DIR}/win/Laf.h.in
)

target_sources_conditional(${ExecutableTarget}
   FLAG APPLE
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/macx/PlatformImplementation.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXFunctions.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXSystemServiceTask.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MEGAService.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/ClientSide.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/ServerSide.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXExtServer.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalServer.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalServerPrivate.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalSocket.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalSocketPrivate.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NSPopover+MISSINGBackgroundView.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/Protocol.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXExtServerService.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NativeMacPopover.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NativeMacPopoverPrivate.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/PlatformImplementation.cpp
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXExtServerService.cpp
   ${CMAKE_CURRENT_LIST_DIR}/macx/PlatformStrings.cpp
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXFunctions.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXSystemServiceTask.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MEGAService.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/ClientSide.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/ServerSide.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXExtServer.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalServer.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalServerPrivate.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalSocket.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/MacXLocalSocketPrivate.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/NSPopover+MISSINGBackgroundView.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/PowerOptions.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/NativeMacPopover.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/NativeMacPopoverPrivate.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/LockedPopOver.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/LockedPopOver.mm
)

target_sources_conditional(${ExecutableTarget}
   FLAG UNIX AND NOT APPLE
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/linux/PlatformImplementation.h
   ${CMAKE_CURRENT_LIST_DIR}/linux/ExtServer.h
   ${CMAKE_CURRENT_LIST_DIR}/linux/NotifyServer.h
   ${CMAKE_CURRENT_LIST_DIR}/linux/DolphinFileManager.h
   ${CMAKE_CURRENT_LIST_DIR}/linux/NautilusFileManager.h
   ${CMAKE_CURRENT_LIST_DIR}/linux/PlatformImplementation.cpp
   ${CMAKE_CURRENT_LIST_DIR}/linux/ExtServer.cpp
   ${CMAKE_CURRENT_LIST_DIR}/linux/NotifyServer.cpp
   ${CMAKE_CURRENT_LIST_DIR}/linux/PowerOptions.cpp
   ${CMAKE_CURRENT_LIST_DIR}/linux/PlatformStrings.cpp
   ${CMAKE_CURRENT_LIST_DIR}/linux/DolphinFileManager.cpp
   ${CMAKE_CURRENT_LIST_DIR}/linux/NautilusFileManager.cpp
)

if (WIN32)
    target_compile_definitions(${ExecutableTarget}
        PUBLIC
        _UNICODE
    )

    target_link_libraries(${ExecutableTarget}
        PRIVATE
        Shell32 Shlwapi Powrprof taskschd
    )
elseif (APPLE)
    target_link_libraries(${ExecutableTarget}
        PRIVATE
        "-framework IOKit"
    )
else ()
    find_package(Qt5 REQUIRED COMPONENTS X11Extras DBus)
    target_link_libraries(${ExecutableTarget}
        PRIVATE
        Qt5::X11Extras
        xcb
        Qt5::DBus
    )
    target_compile_definitions(${ExecutableTarget}
        PUBLIC
        USE_DBUS
    )
endif()

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_PLATFORM_HEADERS}
    ${DESKTOP_APP_PLATFORM_SOURCES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/linux
    ${CMAKE_CURRENT_LIST_DIR}/mac
    ${CMAKE_CURRENT_LIST_DIR}/win
    ${CMAKE_CURRENT_BINARY_DIR}/win
)
target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})

if (UNIX AND NOT APPLE AND NOT DontUseResources)

   # Install app icons
   install(DIRECTORY platform/linux/data/icons
       DESTINATION "${CMAKE_INSTALL_BINDIR}/../share"
   )
   # Install .desktop
   install(FILES platform/linux/data/megasync.desktop
       DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/applications"
   )

endif()
