
set(DESKTOP_APP_PLATFORM_HEADERS
    platform/Platform.h
    platform/AbstractPlatform.h
    platform/ShellNotifier.h
    platform/PowerOptions.h
    platform/PlatformStrings.h
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
)

set(DESKTOP_APP_PLATFORM_SOURCES
    platform/AbstractPlatform.cpp
    platform/Platform.cpp
    platform/ShellNotifier.cpp
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
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   PRIVATE
   platform/macx/LockedPopOver.h
   platform/macx/LockedPopOver.mm
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_PLATFORM_HEADERS}
    ${DESKTOP_APP_PLATFORM_SOURCES}
)

target_link_libraries(MEGAsync
    PRIVATE
    "-framework IOKit"
)

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/platform)
