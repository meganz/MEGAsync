
set(DESKTOP_APP_SYNCS_HEADERS
    syncs/gui/Backups/AddBackupDialog.h
    syncs/gui/Backups/BackupSettingsElements.h
    syncs/gui/Backups/BackupNameConflictDialog.h
    syncs/gui/Backups/BackupRenameWidget.h
    syncs/gui/Backups/BackupTableView.h
    syncs/gui/Backups/BackupsWizard.h
    syncs/gui/Backups/RemoveBackupDialog.h
    syncs/gui/Backups/BackupSettingsUI.h
    syncs/gui/SyncSettingsUIBase.h
    syncs/gui/SyncTooltipCreator.h
    syncs/gui/SyncsMenu.h
    syncs/gui/Twoways/BindFolderDialog.h
    syncs/gui/Twoways/IgnoresEditingDialog.h
    syncs/gui/Twoways/FolderBinder.h
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
)

set(DESKTOP_APP_SYNCS_SOURCES
    syncs/gui/Backups/AddBackupDialog.cpp
    syncs/gui/Backups/BackupSettingsElements.cpp
    syncs/gui/Backups/BackupNameConflictDialog.cpp
    syncs/gui/Backups/BackupRenameWidget.cpp
    syncs/gui/Backups/BackupTableView.cpp
    syncs/gui/Backups/BackupsWizard.cpp
    syncs/gui/Backups/RemoveBackupDialog.cpp
    syncs/gui/Backups/BackupSettingsUI.cpp
    syncs/gui/SyncSettingsUIBase.cpp
    syncs/gui/SyncTooltipCreator.cpp
    syncs/gui/SyncsMenu.cpp
    syncs/gui/Twoways/BindFolderDialog.cpp
    syncs/gui/Twoways/IgnoresEditingDialog.cpp
    syncs/gui/Twoways/FolderBinder.cpp
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
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   PRIVATE
   syncs/../gui/Resources_macx.qrc
   syncs/gui/Twoways/macx/FolderBinder.ui
   syncs/gui/Twoways/macx/BindFolderDialog.ui
   syncs/gui/Twoways/macx/IgnoresEditingDialog.ui
   syncs/gui/Twoways/macx/SyncAccountFullMessage.ui
   syncs/gui/Twoways/macx/SyncSettingsUIBase.ui
   syncs/gui/Twoways/macx/SyncStallModeSelector.ui
   syncs/gui/Backups/macx/BackupsWizard.ui
   syncs/gui/Backups/macx/AddBackupDialog.ui
   syncs/gui/Backups/macx/RemoveBackupDialog.ui
   syncs/gui/Backups/macx/BackupNameConflictDialog.ui
   syncs/gui/Backups/macx/BackupRenameWidget.ui
   syncs/gui/Backups/macx/OpenBackupsFolder.ui
)

if (APPLE)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        syncs/gui/Twoways/macx
        syncs/gui/Backups/macx
    )
else()
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        syncs/gui/Twoways/win
        syncs/gui/Backups/win
    )
endif()

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_SYNCS_HEADERS}
    ${DESKTOP_APP_SYNCS_SOURCES}
)

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR})
