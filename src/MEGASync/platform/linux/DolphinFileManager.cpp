#include "DolphinFileManager.h"
#include <QProcess>

const QString DolphinFileManager::ShowSelectedInFolderParam = QLatin1String("--select");
const QString DolphinFileManager::OpenNewWindowParam = QLatin1String("--new-window");

QStringList DolphinFileManager::getShowInFolderParams()
{
    return QStringList() << OpenNewWindowParam << ShowSelectedInFolderParam;
}

//!
//! \brief DolphinFileManager::changeFolderIcon
//! \param folderPath: path to the folder whose icon is to be changed.
//! \param iconPath: path to the new icon to be set for the folder.
//! \If empty, the default icon is restored.
//! \Changes the icon of the specified folder in Dolphin file manager.
//! \If the folder path is empty, the function returns immediately.
//! \The function attempts to use `kwriteconfig5` and `kwriteconfig6` to update the icon.
void DolphinFileManager::changeFolderIcon(const QString& folderPath, const QString& iconPath)
{
    if (folderPath.isEmpty()) { return; }

    QStringList programs = { QString::fromLatin1("kwriteconfig5"),
                            QString::fromLatin1("kwriteconfig6") };

    // Construct the command to execute
    QStringList arguments;
    arguments << QString::fromLatin1("--file") << folderPath + QString::fromLatin1("/.directory")
              << QString::fromLatin1("--group") << QString::fromLatin1("Desktop Entry")
              << QString::fromLatin1("--key") << QString::fromLatin1("Icon")
              << iconPath;

    // Execute the command
    for (const QString& program : programs)
    {
        QProcess process;
        process.start(program, arguments);
        process.waitForFinished();
    }
}
