#ifndef STALLEDISSUESUTILITIES_H
#define STALLEDISSUESUTILITIES_H

#include "StalledIssue.h"

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

    void removeRemoteFile(const QString& path);
    void removeRemoteFile(mega::MegaNode* node);
    void removeLocalFile(const QString& path, const mega::MegaHandle &syncId);

    static QIcon getLocalFileIcon(const QFileInfo& fileInfo, bool hasProblem);
    static QIcon getRemoteFileIcon(mega::MegaNode* node, const QFileInfo &fileInfo, bool hasProblem);
    static QIcon getIcon(bool isFile, const QFileInfo &fileInfo, bool hasProblem);

signals:
    void actionFinished();

private:
    mutable QReadWriteLock  mIgnoreMutex;
};

/////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
class MegaDownloader;

class FingerprintMissingSolver : public QObject
{
public:
    FingerprintMissingSolver();

    void solveIssues(const QList<StalledIssueVariant>& pathsToSolve);

private:
    std::unique_ptr<MegaDownloader> mDownloader;
};

#endif // STALLEDISSUESUTILITIES_H
