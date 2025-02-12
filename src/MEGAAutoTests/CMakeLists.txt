
add_library(trompeloeil INTERFACE)
target_include_directories(trompeloeil INTERFACE "3rdparty/trompeloeil")

set(MegaSyncDir "../../MEGASync")

set(MEGA_DESKTOP_APP_SOURCES
    ${MegaSyncDir}/BlockingStageProgressController.cpp
    ${MegaSyncDir}/CommonMessages.cpp
    ${MegaSyncDir}/EventUpdater.cpp
    ${MegaSyncDir}/FolderTransferListener.cpp
    ${MegaSyncDir}/MegaApplication.cpp
    ${MegaSyncDir}/ScaleFactorManager.cpp
    ${MegaSyncDir}/ScanStageController.cpp
    ${MegaSyncDir}/TransferQuota.cpp
    ${MegaSyncDir}/UserAlertTimedClustering.cpp
    ${MegaSyncDir}/drivedata.cpp
)

add_subdirectory(UnitTests)
