
set(DESKTOP_APP_SYNCS_HEADERS
    syncs/gui/Backups/RemoveBackupDialog.h
    syncs/gui/SyncTooltipCreator.h
    syncs/gui/SyncsMenu.h
    syncs/gui/Twoways/RemoveSyncConfirmationDialog.h
    syncs/control/MegaIgnoreManager.h
    syncs/control/MegaIgnoreRules.h
    syncs/control/SyncController.h
    syncs/control/SyncInfo.h
    syncs/control/SyncSettings.h
    syncs/control/CreateRemoveSyncsManager.h
    syncs/control/CreateRemoveBackupsManager.h
)

set(DESKTOP_APP_SYNCS_SOURCES
    syncs/gui/Backups/RemoveBackupDialog.cpp
    syncs/gui/SyncTooltipCreator.cpp
    syncs/gui/SyncsMenu.cpp
    syncs/gui/Twoways/RemoveSyncConfirmationDialog.cpp
    syncs/control/MegaIgnoreManager.cpp
    syncs/control/MegaIgnoreRules.cpp
    syncs/control/SyncInfo.cpp
    syncs/control/SyncController.cpp
    syncs/control/SyncSettings.cpp
    syncs/control/CreateRemoveSyncsManager.cpp
    syncs/control/CreateRemoveBackupsManager.cpp
)

target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   syncs/gui/Twoways/win/SyncAccountFullMessage.ui
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   syncs/gui/Twoways/macx/SyncAccountFullMessage.ui
)

target_sources_conditional(MEGAsync
   FLAG UNIX AND NOT APPLE
   QT_AWARE
   PRIVATE
   syncs/gui/Twoways/linux/SyncAccountFullMessage.ui
)

if (WIN32)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        syncs/gui/Twoways/win
        syncs/gui/Backups/win
    )
elseif (APPLE)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        syncs/gui/Twoways/macx
        syncs/gui/Backups/macx
    )
else()
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        syncs/gui/Twoways/linux
        syncs/gui/Backups/linux
    )
endif()

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_SYNCS_HEADERS}
    ${DESKTOP_APP_SYNCS_SOURCES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways
    ${CMAKE_CURRENT_LIST_DIR}/control
)

target_include_directories(MEGAsync PRIVATE ${INCLUDE_DIRECTORIES})
