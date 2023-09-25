#ifndef DOLPHIN_FILEMANAGER_H
#define DOLPHIN_FILEMANAGER_H

#include <QStringList>

class DolphinFileManager
{
    static const QString ShowSelectedInFolderParam;
    static const QString OpenNewWindowParam;

public:
    static QStringList getShowInFolderParams();
};

#endif
