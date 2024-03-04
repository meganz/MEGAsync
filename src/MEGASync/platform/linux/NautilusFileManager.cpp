#include "NautilusFileManager.h"

const QString NautilusFileManager::OpenNewWindowParam = QLatin1String("--new-window");

QStringList NautilusFileManager::getShowInFolderParams()
{
    return QStringList() << OpenNewWindowParam;
}
