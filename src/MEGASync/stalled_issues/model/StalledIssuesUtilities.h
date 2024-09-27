#ifndef STALLEDISSUESUTILITIES_H
#define STALLEDISSUESUTILITIES_H

#include "StalledIssue.h"

#include <megaapi.h>
#include <MegaApplication.h>
#include <Utilities.h>
#include "SyncInfo.h"
#include <TextDecorator.h>

#include <QObject>
#include <QString>
#include <QFutureWatcher>
#include <QFileInfo>
#include <QReadWriteLock>
#include <QEventLoop>

#include <memory>

class StalledIssuesBoldTextDecorator
{
public:
    StalledIssuesBoldTextDecorator() = default;
    static const Text::Decorator boldTextDecorator;
};

class StalledIssuesNewLineTextDecorator
{
public:
    StalledIssuesNewLineTextDecorator() = default;

    static const Text::Decorator newLineTextDecorator;
};

class StalledIssuesLinkTextDecorator
{
public:
    StalledIssuesLinkTextDecorator() = default;

    static void process(const QStringList& links, QString& input)
    {
        Text::Decorator linkTextDecorator(new Text::Link(links));
        linkTextDecorator.process(input);
    }
};

class StalledIssuesUtilities : public QObject
{
    Q_OBJECT

public:
    StalledIssuesUtilities();

    static std::shared_ptr<mega::MegaError> removeRemoteFile(const QString& path);
    static std::shared_ptr<mega::MegaError> removeRemoteFile(mega::MegaNode* node);
    static bool removeLocalFile(const QString& path, const mega::MegaHandle& syncId);

    static QIcon getLocalFileIcon(const QFileInfo& fileInfo, bool hasProblem);
    static QIcon getRemoteFileIcon(mega::MegaNode* node, const QFileInfo &fileInfo, bool hasProblem);
    static QIcon getIcon(bool isFile, const QFileInfo &fileInfo, bool hasProblem);

    static void openLink(bool isCloud, const QString& path);
    static QString getLink(bool isCloud, const QString& path);

    struct KeepBothSidesState
    {
        enum Side
        {
            NONE,
            LOCAL,
            REMOTE
        };

        Side sideRenamed = Side::NONE;
        QString newName;
        std::shared_ptr<mega::MegaError> error;
    };

    static KeepBothSidesState KeepBothSides(std::shared_ptr<mega::MegaNode> node,
                                            const QString& localFilePath);
    static MegaDownloader* getMegaDownloader();
    static MegaUploader* getMegaUploader();

signals:
    void actionFinished();

private:
    mutable QReadWriteLock  mIgnoreMutex;
    static std::unique_ptr<MegaDownloader> mDownloader;
    static std::unique_ptr<MegaUploader> mUploader;
};

/////////////////////////////////////////////////////////////////////////////////////////
class StalledIssuesBySyncFilter
{
public:
    StalledIssuesBySyncFilter(){}

    static void resetFilter();
    static QSet<mega::MegaHandle> getSyncIdsByStall(const mega::MegaSyncStall* stall);

private:
    static mega::MegaHandle filterByPath(const QString& path, bool cloud);
    static bool isBelow(mega::MegaHandle syncRootNode, mega::MegaHandle checkNode);
    static bool isBelow(const QString& syncRootPath, const QString& checkPath);

    static QMap<QVariant, mega::MegaHandle> mSyncIdCache;
    static QHash<const mega::MegaSyncStall*, QSet<mega::MegaHandle>> mSyncIdCacheByStall;
};

/////////////////////////////////////////////////////////////////////////////////////////
class MegaDownloader;

class FingerprintMissingSolver : public QObject
{
public:
    FingerprintMissingSolver();

    void solveIssues(const QList<StalledIssueVariant>& pathsToSolve);
};

#endif // STALLEDISSUESUTILITIES_H
