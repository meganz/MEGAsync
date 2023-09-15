#ifndef ISYSTEMAPPLICATIONMANAGER_H
#define ISYSTEMAPPLICATIONMANAGER_H

#include <QString>

class ISystemApplicationManager
{
    public:
        virtual QString getShowInFolderParams() const = 0;
        virtual ~ISystemApplicationManager() = default;
};

#endif // ISYSTEMAPPLICATIONMANAGER_H
