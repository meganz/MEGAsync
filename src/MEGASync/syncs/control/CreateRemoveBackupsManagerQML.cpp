#include "CreateRemoveBackupsManagerQML.h"

#include "CreateRemoveBackupsManager.h"
#include "SyncSettings.h"

void CreateRemoveBackupsManagerQML::addBackup(bool comesFromSettings)
{
    CreateRemoveBackupsManager::addBackup(comesFromSettings);
}

void CreateRemoveBackupsManagerQML::removeBackup(std::shared_ptr<SyncSettings> backup,
                                                 QWidget* parent)
{
    CreateRemoveBackupsManager::removeBackup(backup, parent);
}
