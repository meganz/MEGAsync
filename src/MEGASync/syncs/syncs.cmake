
set(DESKTOP_APP_SYNCS_HEADERS
    syncs/gui/Backups/BackupSettingsElements.h
    syncs/gui/Backups/BackupTableView.h
    syncs/gui/Backups/RemoveBackupDialog.h
    syncs/gui/Backups/BackupSettingsUI.h
    syncs/gui/SyncSettingsUIBase.h
    syncs/gui/SyncTooltipCreator.h
    syncs/gui/SyncsMenu.h
    syncs/gui/Twoways/RemoveSyncConfirmationDialog.h
    syncs/gui/Twoways/SyncTableView.h
    syncs/gui/Twoways/SyncSettingsUI.h
    syncs/gui/Twoways/SyncSettingsElements.h
    syncs/model/BackupItemModel.h
    syncs/model/SyncItemModel.h
    syncs/control/MegaIgnoreManager.h
    syncs/control/MegaIgnoreRules.h
    syncs/control/SyncController.h
    syncs/control/SyncInfo.h
    syncs/control/SyncSettings.h
    syncs/control/AddSyncFromUiManager.h
    syncs/control/AddBackupFromUiManager.h
)

set(DESKTOP_APP_SYNCS_SOURCES
    syncs/gui/Backups/BackupSettingsElements.cpp
    syncs/gui/Backups/BackupTableView.cpp
    syncs/gui/Backups/RemoveBackupDialog.cpp
    syncs/gui/Backups/BackupSettingsUI.cpp
    syncs/gui/SyncSettingsUIBase.cpp
    syncs/gui/SyncTooltipCreator.cpp
    syncs/gui/SyncsMenu.cpp
    syncs/gui/Twoways/RemoveSyncConfirmationDialog.cpp
    syncs/gui/Twoways/SyncTableView.cpp
    syncs/gui/Twoways/SyncSettingsUI.cpp
    syncs/gui/Twoways/SyncSettingsElements.cpp
    syncs/model/BackupItemModel.cpp
    syncs/model/SyncItemModel.cpp
    syncs/control/MegaIgnoreManager.cpp
    syncs/control/MegaIgnoreRules.cpp
    syncs/control/SyncInfo.cpp
    syncs/control/SyncController.cpp
    syncs/control/SyncSettings.cpp
    syncs/control/AddSyncFromUiManager.cpp
    syncs/control/AddBackupFromUiManager.cpp
)

target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   syncs/gui/Twoways/win/RemoveSyncConfirmationDialog.ui
   syncs/gui/Twoways/win/SyncAccountFullMessage.ui
   syncs/gui/Twoways/win/SyncSettingsUIBase.ui
   syncs/gui/Twoways/win/SyncStallModeSelector.ui
   syncs/gui/Backups/win/RemoveBackupDialog.ui
   syncs/gui/Backups/win/OpenBackupsFolder.ui
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   syncs/gui/Twoways/macx/RemoveSyncConfirmationDialog.ui
   syncs/gui/Twoways/macx/SyncAccountFullMessage.ui
   syncs/gui/Twoways/macx/SyncSettingsUIBase.ui
   syncs/gui/Twoways/macx/SyncStallModeSelector.ui
   syncs/gui/Backups/macx/RemoveBackupDialog.ui
   syncs/gui/Backups/macx/OpenBackupsFolder.ui
)

target_sources_conditional(MEGAsync
   FLAG UNIX AND NOT APPLE
   QT_AWARE
   PRIVATE
   syncs/gui/Twoways/linux/RemoveSyncConfirmationDialog.ui
   syncs/gui/Twoways/linux/SyncAccountFullMessage.ui
   syncs/gui/Twoways/linux/SyncSettingsUIBase.ui
   syncs/gui/Twoways/linux/SyncStallModeSelector.ui
   syncs/gui/Backups/linux/RemoveBackupDialog.ui
   syncs/gui/Backups/linux/OpenBackupsFolder.ui
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

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR})
