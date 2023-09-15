#ifndef NAUTILUS_FILEMANAGER_H
#define NAUTILUS_FILEMANAGER_H

#include <string>

#include "ISystemApplicationManager.h"

class NautilusFileManager : public ISystemApplicationManager
{
public:
    QString getShowInFolderParams() const override;

private:
    static bool registered;
};

#endif
