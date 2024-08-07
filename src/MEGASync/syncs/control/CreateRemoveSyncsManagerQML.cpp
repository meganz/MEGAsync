#include "CreateRemoveSyncsManagerQML.h"

#include "CreateRemoveSyncsManager.h"
#include "SyncSettings.h"

void CreateRemoveSyncsManagerQML::addBackup(bool comesFromSettings)
{
    CreateRemoveSyncsManager::addSync(comesFromSettings);
}

void CreateRemoveSyncsManagerQML::removeBackup(mega::MegaHandle handle, QWidget* parent)
{
    CreateRemoveSyncsManager::removeSync(handle, parent);
}
