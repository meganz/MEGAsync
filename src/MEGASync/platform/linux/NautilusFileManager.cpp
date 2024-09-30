#include "NautilusFileManager.h"
#include <QProcess>

const QString NautilusFileManager::OpenNewWindowParam = QLatin1String("--new-window");

QStringList NautilusFileManager::getShowInFolderParams()
{
    return QStringList() << OpenNewWindowParam;
}

//!
//! \brief NautilusFileManager::changeFolderIcon
//! \param folderPath: path to the folder whose icon is to be changed.
//! \param iconPath: path to the new icon to be set for the folder.
//! \If empty, the default icon is restored.
//! \Changes the icon of the specified folder in Nautilus file manager.
//! \If the folder path is empty, the function returns immediately.
void NautilusFileManager::changeFolderIcon(const QString& folderPath, const QString& iconPath)
{
    if (folderPath.isEmpty()) { return; }

    QStringList arguments = { QString::fromLatin1("set"), QString::fromLatin1("-t") };
    QStringList arguments2 = { QString::fromLatin1("%1").arg(folderPath),
                               QString::fromLatin1("metadata::custom-icon") };

    if (iconPath.isEmpty())
    {
        // Remove the icon
        arguments << QString::fromLatin1("unset") << arguments2;
    }
    else
    {
        // Add the icon
        arguments << QString::fromLatin1("string") << arguments2
                  << QString::fromLatin1("file://%1").arg(iconPath);
    }

    QProcess process;
    process.start(QString::fromLatin1("gio"), arguments);
    process.waitForFinished();
}
