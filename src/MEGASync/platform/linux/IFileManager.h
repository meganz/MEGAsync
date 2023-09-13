#ifndef IFILEMANAGER_H
#define IFILEMANAGER_H

#include <QString>

class IFileManager
{
    public:
        virtual QString getShowInFolderParams() const = 0;
};

#endif // IFILEMANAGER_H
