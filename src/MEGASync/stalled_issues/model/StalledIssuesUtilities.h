#ifndef STALLEDISSUESUTILITIES_H
#define STALLEDISSUESUTILITIES_H

#include <megaapi.h>
#include <MegaApplication.h>
#include <Utilities.h>
#include <syncs/control/SyncInfo.h>

#include <QObject>
#include <QString>
#include <QFutureWatcher>
#include <QFileInfo>
#include <QReadWriteLock>
#include <QEventLoop>

#include <memory>


class StalledIssuesUtilities : public QObject
{
    Q_OBJECT

public:
    StalledIssuesUtilities();

    void ignoreFile(const QString& path);
    void ignoreSymLinks(const QString &path);

    void removeRemoteFile(const QString& path);
    void removeRemoteFile(mega::MegaNode* node);
    void removeLocalFile(const QString& path);

    static QIcon getLocalFileIcon(const QFileInfo& fileInfo, bool hasProblem);
    static QIcon getRemoteFileIcon(mega::MegaNode* node, const QFileInfo &fileInfo, bool hasProblem);
    static QIcon getIcon(bool isFile, const QFileInfo &fileInfo, bool hasProblem);

signals:
    void actionFinished();
    void remoteActionFinished(mega::MegaHandle handle);

private:
    mutable QReadWriteLock  mIgnoreMutex;

    QList<mega::MegaHandle> mRemoteHandles;
};

//Only used when there is no local info -> Otherwise, use MegaApi::getSyncedNode(localPath)
class StalledIssuesBySyncFilter
{
public:
    StalledIssuesBySyncFilter(){}

    void resetFilter(){mSyncIdCache.clear();}

    mega::MegaHandle filterByPath(const QString& path, bool cloud);

private:
    bool isBelow(mega::MegaHandle syncRootNode, mega::MegaHandle checkNode);
    bool isBelow(const QString& syncRootPath, const QString& checkPath);

    static QMap<QVariant, mega::MegaHandle> mSyncIdCache;
};

#endif // STALLEDISSUESUTILITIES_H
