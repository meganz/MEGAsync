#ifndef DOLPHINFILEMANAGER_H
#define DOLPHINFILEMANAGER_H

#include <string>

#include "IFileManager.h"
#include "FileManagerFactory.h"

class DolphinFileManager : public IFileManager
{
public:
    QString getShowInFolderParams() const override;

private:
    static bool registered;
};

#endif
