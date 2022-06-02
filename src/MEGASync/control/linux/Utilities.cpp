#include <unistd.h>

#include <QStorageInfo>
#include <QStandardPaths>

#include "Utilities.h"

bool Utilities::moveFileToTrash(const QString &filePath)
{
   const QFileInfo sourceInfo(filePath);
   const QString sourcePath = sourceInfo.absolutePath();

   QDir trashDir(linuxTrashLocation(sourcePath));
   if (!trashDir.exists())
   {
       return false;
   }
   const QLatin1String filesDir("files");
   const QLatin1String infoDir("info");
   trashDir.mkdir(filesDir);
   int savedErrno = errno;
   trashDir.mkdir(infoDir);
   if (!savedErrno)
       savedErrno = errno;
   if (!trashDir.exists(filesDir) || !trashDir.exists(infoDir))
   {
       return false;
   }

   const QString trashedName = sourceInfo.isDir()
                             ? QDir(sourcePath).dirName()
                             : sourceInfo.fileName();
   QString uniqueTrashedName = QLatin1Char('/') + trashedName;
   QString infoFileName;
   int counter = 0;
   QFile infoFile;
   auto makeUniqueTrashedName = [trashedName, &counter]() -> QString {
       ++counter;
       return QString(QLatin1String("/%1-%2"))
                                       .arg(trashedName)
                                       .arg(counter, 4, 10, QLatin1Char('0'));
   };
   do {
       while (QFile::exists(trashDir.filePath(filesDir) + uniqueTrashedName))
           uniqueTrashedName = makeUniqueTrashedName();
       infoFileName = trashDir.filePath(infoDir)
                    + uniqueTrashedName + QLatin1String(".trashinfo");
       infoFile.setFileName(infoFileName);
       if (!infoFile.open(QIODevice::NewOnly | QIODevice::WriteOnly | QIODevice::Text))
           uniqueTrashedName = makeUniqueTrashedName();
   } while (!infoFile.isOpen());

   const QString targetPath = trashDir.filePath(filesDir) + uniqueTrashedName;
   infoFile.close();

   return sourceInfo.dir().rename(sourceInfo.filePath(),targetPath);
}

QDir Utilities::linuxTrashLocation(const QString &sourcePath)
{
    auto makeTrashDir = [](const QDir &topDir, const QString &trashDir) -> QString {
            auto ownerPerms = QFileDevice::ReadOwner
                            | QFileDevice::WriteOwner
                            | QFileDevice::ExeOwner;
            QString targetDir = topDir.filePath(trashDir);

            if (topDir.mkdir(trashDir))
                QFile::setPermissions(targetDir, ownerPerms);
            if (QFileInfo(targetDir).isDir())
                return targetDir;
            return QString();
        };
        auto isSticky = [](const QFileInfo &fileInfo) -> bool {
            struct stat st;
            if (stat(QFile::encodeName(fileInfo.absoluteFilePath()).constData(), &st) == 0)
                return st.st_mode & S_ISVTX;

            return false;
        };

        QString trash;
        const QStorageInfo sourceStorage(sourcePath);
        const QStorageInfo homeStorage(QDir::home());

        if (sourceStorage != homeStorage) {
            const auto dotTrash = QString::fromUtf8(".Trash");
            QDir topDir(sourceStorage.rootPath());
            const QString userID = QString::number(::getuid());
            if (topDir.cd(dotTrash)) {
                const QFileInfo trashInfo(topDir.path());
                if (trashInfo.isSymLink()) {
                    qCritical("Warning: '%s' is a symlink to '%s'",
                              trashInfo.absoluteFilePath().toLocal8Bit().constData(),
                              trashInfo.symLinkTarget().toLatin1().constData());
                } else if (!isSticky(trashInfo)) {
                    qCritical("Warning: '%s' doesn't have sticky bit set!",
                              trashInfo.absoluteFilePath().toLocal8Bit().constData());
                } else if (trashInfo.isDir()) {
                    trash = makeTrashDir(topDir, userID);
                }
            }

            if (trash.isEmpty()) {
                topDir = QDir(sourceStorage.rootPath());
                const QString userTrashDir = dotTrash + u'-' + userID;
                trash = makeTrashDir(topDir, userTrashDir);
            }
        }

        if (trash.isEmpty()) {
            QDir topDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
            trash = makeTrashDir(topDir, QString::fromUtf8("Trash"));
            if (!QFileInfo(trash).isDir()) {
                qWarning("Unable to establish trash directory in %s",
                         topDir.path().toLocal8Bit().constData());
            }
        }

        return trash;
}

