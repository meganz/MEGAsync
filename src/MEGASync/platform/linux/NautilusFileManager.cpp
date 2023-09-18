#include "NautilusFileManager.h"

static const QString OpenNewWindow = QLatin1String("--new-window");

QStringList NautilusFileManager::getShowInFolderParams()
{
    return QStringList() << OpenNewWindow;
}
