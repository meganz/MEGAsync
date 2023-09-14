#ifndef ISYSTEMAPPLICATIONMANAGER_H
#define ISYSTEMAPPLICATIONMANAGER_H

#include <QString>

class ISystemApplicationManager
{
    public:
        virtual QString getShowInFolderParams() const = 0;
};

#endif // ISYSTEMAPPLICATIONMANAGER_H
