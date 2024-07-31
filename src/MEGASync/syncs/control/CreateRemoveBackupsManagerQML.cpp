#include "CreateRemoveBackupsManagerQML.h"

#include "CreateRemoveBackupsManager.h"
#include "SyncSettings.h"


const CreateRemoveBackupsManager* const CreateRemoveBackupsManagerQML::addBackup(
    bool comesFromSettings)
{
    return CreateRemoveBackupsManager::addBackup(comesFromSettings);
}

const CreateRemoveBackupsManager* const CreateRemoveBackupsManagerQML::removeBackup(
    std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    return CreateRemoveBackupsManager::removeBackup(backup, parent);
}
