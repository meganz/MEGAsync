#ifndef NAUTILUS_FILEMANAGER_H
#define NAUTILUS_FILEMANAGER_H

#include <QStringList>

class NautilusFileManager
{
    static const QString OpenNewWindowParam;

public:
    static QStringList getShowInFolderParams();
    static void changeFolderIcon(const QString& folderPath, const QString& iconPath = QString());
};

#endif
