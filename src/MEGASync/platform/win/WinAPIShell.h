#ifndef WINAPISHELL_H
#define WINAPISHELL_H

#include "ShellNotifier.h"

#include <Shlobj.h>
#include <string>

class WindowsApiShellNotifier : public AbstractShellNotifier
{
public :
    void notify(const QString& path) override
    {
        logNotify("WindowsAPIShellNotifier", path);
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, path.utf16(), NULL);
    }
};

#endif // WINAPISHELL_H
