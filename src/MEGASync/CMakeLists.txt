#
# MEGA Desktop App
#

add_executable(MEGAsync MACOSX_BUNDLE)

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

target_sources(MEGAsync
    PRIVATE
    ${MEGA_DESKTOP_APP_HEADERS}
    ${MEGA_DESKTOP_APP_SOURCES}
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

# Load and link needed libraries for the Desktop App target
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Network MacExtras Qml Quick)
message(STATUS "Building the MEGA Desktop App with Qt v${Qt5_VERSION} from ${Qt5_DIR}")

# Load and link needed libraries for the Desktop App target
target_link_libraries(MEGAsync
    PRIVATE    
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::MacExtras
    Qt5::Qml
    Qt5::Quick
    MEGA::SDKlib
    MEGA::SDKQtBindings
    MEGA::google_breakpad
)
