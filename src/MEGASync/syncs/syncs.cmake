
set(DESKTOP_APP_SYNCS_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/RemoveBackupDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncTooltipCreator.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncsMenu.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/RemoveSyncConfirmationDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreManager.h
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreRules.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncController.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncControllerInternal.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveSyncsManager.h
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveBackupsManager.h
    ${CMAKE_CURRENT_LIST_DIR}/control/RemoveBackup.h
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/SyncReminderNotificationManager.h
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/SyncReminderAction.h
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/FirstSyncReminderAction.h
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/MultiSyncReminderAction.h
)

set(DESKTOP_APP_SYNCS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/RemoveBackupDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncTooltipCreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/SyncsMenu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways/RemoveSyncConfirmationDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/MegaIgnoreRules.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/SyncSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveSyncsManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/CreateRemoveBackupsManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/RemoveBackup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/SyncReminderNotificationManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/SyncReminderAction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/FirstSyncReminderAction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders/MultiSyncReminderAction.cpp
)

set (DESKTOP_APP_SYNCS_UI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/ui/RemoveBackupDialog.ui
)

set_property(TARGET ${ExecutableTarget}
    APPEND PROPERTY AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups/ui
)

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_SYNCS_HEADERS}
    ${DESKTOP_APP_SYNCS_SOURCES}
    ${DESKTOP_APP_SYNCS_UI_FILES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/gui/Backups
    ${CMAKE_CURRENT_LIST_DIR}/gui/Twoways
    ${CMAKE_CURRENT_LIST_DIR}/control
    ${CMAKE_CURRENT_LIST_DIR}/control/reminders
)

target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})
