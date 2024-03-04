#include "DolphinFileManager.h"

const QString DolphinFileManager::ShowSelectedInFolderParam = QLatin1String("--select");
const QString DolphinFileManager::OpenNewWindowParam = QLatin1String("--new-window");

QStringList DolphinFileManager::getShowInFolderParams()
{
    return QStringList() << OpenNewWindowParam << ShowSelectedInFolderParam;
}
