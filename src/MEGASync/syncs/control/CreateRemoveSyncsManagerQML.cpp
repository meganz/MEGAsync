#include "CreateRemoveSyncsManagerQML.h"

#include "CreateRemoveSyncsManager.h"
#include "SyncSettings.h"

const CreateRemoveSyncsManager* const CreateRemoveSyncsManagerQML::addBackup(bool comesFromSettings)
{
    return CreateRemoveSyncsManager::addSync(comesFromSettings);
}

const CreateRemoveSyncsManager* const
    CreateRemoveSyncsManagerQML::removeBackup(mega::MegaHandle handle, QWidget* parent)
{
    return CreateRemoveSyncsManager::removeSync(handle, parent);
}
