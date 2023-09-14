#include "NautilusFileManager.h"

bool NautilusFileManager::registered = ConcreteSystemApplicationManagerFactory<NautilusFileManager>::Register("nautilus");

const QString OpenNewWindow = QLatin1String("--new-window");

QString NautilusFileManager::getShowInFolderParams() const
{
    return QString(QLatin1String(" %0")).arg(OpenNewWindow);
}
