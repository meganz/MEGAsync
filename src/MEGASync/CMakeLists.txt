#
# MEGA Desktop App
#

if (WIN32)
    add_executable(MEGAsync WIN32)
elseif (APPLE)
    add_executable(MEGAsync MACOSX_BUNDLE)
else()
endif()

set(MEGA_DESKTOP_APP_HEADERS
    BlockingStageProgressController.h
    CommonMessages.h
    EventUpdater.h
    FolderTransferEvents.h
    FolderTransferListener.h
    MegaApplication.h
    ScaleFactorManager.h
    ScanStageController.h
    TransferQuota.h
    UserAlertTimedClustering.h
    drivedata.h
)

set(MEGA_DESKTOP_APP_SOURCES
    BlockingStageProgressController.cpp
    CommonMessages.cpp
    EventUpdater.cpp
    FolderTransferListener.cpp
    MegaApplication.cpp
    ScaleFactorManager.cpp
    ScanStageController.cpp
    TransferQuota.cpp
    UserAlertTimedClustering.cpp
    drivedata.cpp
    main.cpp
)

if (WIN32)
    set(RESOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/icon.rc
    )
elseif (APPLE)
    set(RESOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/app.icns
        ${CMAKE_CURRENT_SOURCE_DIR}/folder.icns
        ${CMAKE_CURRENT_SOURCE_DIR}/folder_yosemite.icns
        ${CMAKE_CURRENT_SOURCE_DIR}/folder_bigsur.icns
        ${CMAKE_CURRENT_SOURCE_DIR}/appicon32.tiff
    )

    set_target_properties(MEGAsync PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info_MEGA.plist
        RESOURCE "${RESOURCE_FILES}"
    )
else()
endif()

target_sources(MEGAsync
    PRIVATE
    ${MEGA_DESKTOP_APP_HEADERS}
    ${MEGA_DESKTOP_APP_SOURCES}
    ${RESOURCE_FILES}
)

# Activate properties for Qt code
set_target_properties(MEGAsync
    PROPERTIES
    AUTOUIC ON # Activates the User Interface Compiler generator for Qt.
    AUTOMOC ON # Activates the meta-object code generator for Qt.
    AUTORCC ON # Activates the creation of rules over qrc files
)

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/google_breakpad
)

# Include modules in subdirectories with sources for Desktop App
add_subdirectory(google_breakpad)

include(control/control.cmake)
include(gui/gui.cmake)
include(notifications/notifications.cmake)
include(platform/platform.cmake)
include(qtlockedfile/qtlockedfile.cmake)
include(stalled_issues/stalledissues.cmake)
include(syncs/syncs.cmake)
include(transfers/transfers.cmake)
include(UserAttributesRequests/userattributesrequests.cmake)

target_compile_definitions(MEGAsync
    PUBLIC
    $<$<CONFIG:Debug>:LOG_TO_STDOUT LOG_TO_LOGGER CREATE_COMPATIBLE_MINIDUMPS>
    $<$<BOOL:${WIN32}>:PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN UNICODE>
)

# Load and link needed libraries for the Desktop App target
if (WIN32)
    find_package(Qt5 REQUIRED COMPONENTS WinExtras)
elseif (APPLE)
    find_package(Qt5 REQUIRED COMPONENTS MacExtras)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Network Qml Quick)
message(STATUS "Building the MEGA Desktop App with Qt v${Qt5_VERSION} from ${Qt5_DIR}")

target_link_libraries(MEGAsync
    PUBLIC
    $<$<BOOL:${WIN32}>:Qt5::WinExtras>
    $<$<BOOL:${APPLE}>:Qt5::MacExtras>
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::Qml
    Qt5::Quick
    MEGA::SDKlib
    MEGA::SDKQtBindings
    MEGA::google_breakpad
)
