#include "DolphinFileManager.h"

bool DolphinFileManager::registered = ConcreteFileManagerFactory<DolphinFileManager>::Register("dolphin");

const QString ShowSelectedInFolder = QLatin1String(" --select");

QString DolphinFileManager::getShowInFolderParams() const
{
    return ShowSelectedInFolder;
}
