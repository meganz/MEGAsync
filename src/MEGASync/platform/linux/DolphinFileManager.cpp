#include "DolphinFileManager.h"

#include "SystemApplicationManagerFactory.h"

bool DolphinFileManager::registered = ConcreteSystemApplicationManagerFactory<DolphinFileManager>::Register("dolphin");

const QString ShowSelectedInFolder = QLatin1String("--select");
const QString OpenNewWindow = QLatin1String("--new-window");

QString DolphinFileManager::getShowInFolderParams() const
{
    return QString(QLatin1String(" %0 %1")).arg(OpenNewWindow, ShowSelectedInFolder);
}
