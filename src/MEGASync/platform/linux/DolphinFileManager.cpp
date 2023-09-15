#include "DolphinFileManager.h"

const QString ShowSelectedInFolder = QLatin1String("--select");
const QString OpenNewWindow = QLatin1String("--new-window");

QString DolphinFileManager::getShowInFolderParams()
{
    return QString(QLatin1String(" %0 %1")).arg(OpenNewWindow, ShowSelectedInFolder);
}
