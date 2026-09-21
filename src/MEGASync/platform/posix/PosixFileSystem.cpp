#include "PosixFileSystem.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <sys/stat.h>

#include <cerrno>
#include <cstring>

namespace PosixFileSystem
{

bool createFolderRestrictedToOwner(const QString& folder, QString* errorMessage)
{
    const auto fail = [errorMessage](const QString& reason)
    {
        if (errorMessage)
        {
            *errorMessage = reason;
        }
        return false;
    };

    if (QDir(folder).exists())
    {
        return true;
    }

    // The parents are shared, standard locations (e.g. the XDG data dir), so they keep
    // their usual permissions; only the leaf is ours to restrict.
    const QString parentFolder = QFileInfo(folder).absolutePath();
    if (!parentFolder.isEmpty() && !QDir().mkpath(parentFolder))
    {
        return fail(
            QString::fromUtf8("the parent folder could not be created (%1)").arg(parentFolder));
    }

    // mkdir(2) applies the mode atomically, so the folder never exists with the default
    // umask, not even briefly. The mode is still masked by the umask, which can only
    // remove permissions, never add them back.
    if (::mkdir(QFile::encodeName(folder).constData(), S_IRWXU) != 0)
    {
        return fail(QString::fromLocal8Bit(strerror(errno)));
    }

    return true;
}

}
