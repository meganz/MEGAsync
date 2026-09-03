#ifndef WINDOWSFILEATTRIBUTEPATH_H
#define WINDOWSFILEATTRIBUTEPATH_H

#include <QDir>
#include <QFileInfo>
#include <QString>

namespace WindowsFileAttributePath
{
inline QString prepare(const QString& path)
{
    constexpr int MAX_PATH_LENGTH = 260;
    const QLatin1String EXTENDED_PREFIX("\\\\?\\");
    const QLatin1String EXTENDED_UNC_PREFIX("\\\\?\\UNC\\");
    const QLatin1String FORWARD_EXTENDED_PREFIX("//?/");
    const QLatin1String FORWARD_EXTENDED_UNC_PREFIX("//?/UNC/");

    QString pathToNormalize(path);
    const bool hadExtendedUncPrefix(
        pathToNormalize.startsWith(EXTENDED_UNC_PREFIX, Qt::CaseInsensitive) ||
        pathToNormalize.startsWith(FORWARD_EXTENDED_UNC_PREFIX, Qt::CaseInsensitive));
    const bool hadExtendedPrefix(hadExtendedUncPrefix ||
                                 pathToNormalize.startsWith(EXTENDED_PREFIX) ||
                                 pathToNormalize.startsWith(FORWARD_EXTENDED_PREFIX));

    // QDir does not understand the Win32 extended-length namespace. Temporarily
    // convert it to a regular local or UNC path so that redundant components can
    // be cleaned safely.
    if (hadExtendedUncPrefix)
    {
        const int prefixSize(pathToNormalize.startsWith(EXTENDED_UNC_PREFIX, Qt::CaseInsensitive) ?
                                 EXTENDED_UNC_PREFIX.size() :
                                 FORWARD_EXTENDED_UNC_PREFIX.size());
        pathToNormalize = QLatin1String("//") + pathToNormalize.mid(prefixSize);
    }
    else if (hadExtendedPrefix)
    {
        const int prefixSize(pathToNormalize.startsWith(EXTENDED_PREFIX) ?
                                 EXTENDED_PREFIX.size() :
                                 FORWARD_EXTENDED_PREFIX.size());
        pathToNormalize.remove(0, prefixSize);
    }

    pathToNormalize = QDir::fromNativeSeparators(pathToNormalize);
    const QString absolutePath(QDir::cleanPath(QFileInfo(pathToNormalize).absoluteFilePath()));
    QString nativePath(QDir::toNativeSeparators(absolutePath));
    if ((hadExtendedPrefix || nativePath.size() >= MAX_PATH_LENGTH) &&
        !nativePath.startsWith(QLatin1String("\\\\?\\")))
    {
        if (nativePath.startsWith(QLatin1String("\\\\")))
        {
            // UNC paths take the \\?\UNC\server\share form.
            nativePath.remove(0, 1);
            nativePath.prepend(QLatin1String("\\\\?\\UNC"));
        }
        else
        {
            nativePath.prepend(QLatin1String("\\\\?\\"));
        }
    }

    return nativePath;
}
} // namespace WindowsFileAttributePath

#endif // WINDOWSFILEATTRIBUTEPATH_H
