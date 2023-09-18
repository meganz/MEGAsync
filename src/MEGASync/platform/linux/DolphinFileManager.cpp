#include "DolphinFileManager.h"

static const QString ShowSelectedInFolder = QLatin1String("--select");
static const QString OpenNewWindow = QLatin1String("--new-window");

QStringList DolphinFileManager::getShowInFolderParams()
{
    return QStringList() << OpenNewWindow << ShowSelectedInFolder;
}
