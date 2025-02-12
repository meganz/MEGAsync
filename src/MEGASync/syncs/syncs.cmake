
set(DESKTOP_APP_SYNCS_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/BackupSettingsElements.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/BackupTableView.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/RemoveBackupDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/BackupSettingsUI.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncSettingsUIBase.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncTooltipCreator.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncsMenu.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/RemoveSyncConfirmationDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/SyncTableView.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/SyncSettingsUI.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/SyncSettingsElements.h
    ${CMAKE_CURRENT_LIST_DIR}/model/BackupItemModel.h
    ${CMAKE_CURRENT_LIST_DIR}/model/SyncItemModel.h
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreManager.h
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreRules.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncController.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveSyncsManager.h
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveBackupsManager.h
)

set(DESKTOP_APP_SYNCS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/BackupSettingsElements.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/BackupTableView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/RemoveBackupDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/BackupSettingsUI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncSettingsUIBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncTooltipCreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncsMenu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/RemoveSyncConfirmationDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/SyncTableView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/SyncSettingsUI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/SyncSettingsElements.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/BackupItemModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/SyncItemModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreRules.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveSyncsManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveBackupsManager.cpp
)

if (WIN32)
    set_property(TARGET ${ExecutableTarget}
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/win
        ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/win
    )
elseif (APPLE)
    set_property(TARGET ${ExecutableTarget}
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/macx
        ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/macx
    )
else()
    set_property(TARGET ${ExecutableTarget}
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/linux
        ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/linux
    )
endif()

target_sources(${ExecutableTarget}
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
    ${CMAKE_CURRENT_LIST_DIR}/model
)

target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})
