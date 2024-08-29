#ifndef DOLPHIN_FILEMANAGER_H
#define DOLPHIN_FILEMANAGER_H

#include <QStringList>

class DolphinFileManager
{
    static const QString ShowSelectedInFolderParam;
    static const QString OpenNewWindowParam;

public:
    static QStringList getShowInFolderParams();
    static void changeFolderIcon(const QString& folderPath, const QString& iconPath = QString());
};

#endif
