#ifndef POSIXFILESYSTEM_H
#define POSIXFILESYSTEM_H

#include <QString>

// Filesystem helpers shared by the Linux and macOS platform implementations, for the
// cases needing POSIX calls that the portable Qt API does not expose. Only built on those
// platforms, so it never has to be guarded for Windows.
namespace PosixFileSystem
{
// Creates `folder`, and any missing parent, so that only its owner can access it from the
// moment it exists. Implements AbstractPlatform::createFolderRestrictedToOwner() for both
// POSIX platforms; see it for why the permissions cannot be applied afterwards. Returns
// false on failure, describing it in `errorMessage` when one is given.
bool createFolderRestrictedToOwner(const QString& folder, QString* errorMessage);
}

#endif // POSIXFILESYSTEM_H
