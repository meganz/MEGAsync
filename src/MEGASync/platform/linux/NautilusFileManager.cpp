#include "NautilusFileManager.h"

const QString OpenNewWindow = QLatin1String("--new-window");

QString NautilusFileManager::getShowInFolderParams()
{
    return QString(QLatin1String(" %0")).arg(OpenNewWindow);
}
