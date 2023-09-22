#include "StalledIssuesUtilities.h"

#include <MegaApplication.h>
#include <mega/types.h>
#include <MegaDownloader.h>

#include <QFile>
#include <QDir>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
StalledIssuesUtilities::StalledIssuesUtilities()
{}

void StalledIssuesUtilities::ignoreFile(const QString &path)
{
    QtConcurrent::run([this, path]()
    {
        QFileInfo tempFile(path);
        QDir ignoreDir(tempFile.path());

        while(ignoreDir.exists())
        {
            QFile ignore(ignoreDir.path() + QDir::separator() + QString::fromUtf8(".megaignore"));
            if(ignore.exists())
            {
                mIgnoreMutex.lockForWrite();
                ignore.open(QFile::Append | QFile::Text);

                QTextStream streamIn(&ignore);
                streamIn.setCodec("UTF-8");

                QString line(QString::fromLatin1("\n-:%1").arg(ignoreDir.relativeFilePath(path)));
                streamIn << line;

                ignore.close();
                mIgnoreMutex.unlock();

                break;
            }

            if(!ignoreDir.cdUp())
            {
                break;
            }
        }

        emit actionFinished();
    });
}

void StalledIssuesUtilities::ignoreSymLinks(const QString& path)
{
    QtConcurrent::run([this, path]()
    {
        QDir ignoreDir(path);
        QFile ignore(ignoreDir.path() + QDir::separator() + QString::fromUtf8(".megaignore"));
        if(ignore.exists())
        {
            mIgnoreMutex.lockForWrite();
            ignore.open(QFile::Append | QFile::Text);

            QTextStream streamIn(&ignore);
            streamIn.setCodec("UTF-8");

            QString line(QString::fromLatin1("\n-s:*"));
            streamIn << line;

            ignore.close();
            mIgnoreMutex.unlock();
            emit actionFinished();
        }
    });
}

void StalledIssuesUtilities::removeRemoteFile(const QString& path)
{
    std::unique_ptr<mega::MegaNode>fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(path.toStdString().c_str()));
    removeRemoteFile(fileNode.get());
}

void StalledIssuesUtilities::removeRemoteFile(mega::MegaNode *node)
{
    if(node)
    {
        mRemoteHandles.append(node->getHandle());
        auto rubbishNode = MegaSyncApp->getMegaApi()->getRubbishNode();
        MegaSyncApp->getMegaApi()->moveNode(node,rubbishNode,
                                            new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(), this, [=](bool isContextValid,
                                                                      const mega::MegaRequest& request,const mega::MegaError& e){
            if(isContextValid)
            {
                if (request.getType() == mega::MegaRequest::TYPE_MOVE
                        || request.getType() == mega::MegaRequest::TYPE_RENAME)
                {
                    if (e.getErrorCode() == mega::MegaError::API_OK)
                    {
                        auto handle = request.getNodeHandle();
                        if(mRemoteHandles.contains(handle))
                        {
                            emit remoteActionFinished(handle);
                            mRemoteHandles.removeOne(handle);
                        }
                    }
                }
            }
        }));
    }
}

void StalledIssuesUtilities::removeLocalFile(const QString& path)
{
    QFile file(path);
    if(file.exists())
    {
         if(Utilities::moveFileToTrash(path))
         {
             emit actionFinished();
         }
    }
}

QIcon StalledIssuesUtilities::getLocalFileIcon(const QFileInfo &fileInfo, bool hasProblem)
{
    bool isFile(false);

    if(fileInfo.exists())
    {
        isFile = fileInfo.isFile();
    }
    else
    {
        isFile = !fileInfo.completeSuffix().isEmpty();
    }

    return getIcon(isFile, fileInfo, hasProblem);
}

QIcon StalledIssuesUtilities::getRemoteFileIcon(mega::MegaNode *node, const QFileInfo& fileInfo, bool hasProblem)
{
    if(node)
    {
        return getIcon(node->isFile(), fileInfo, hasProblem);
    }
    else
    {
        return Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/help-circle.png"));
    }
}

QIcon StalledIssuesUtilities::getIcon(bool isFile, const QFileInfo& fileInfo, bool hasProblem)
{
    QIcon fileTypeIcon;

    if(isFile)
    {
        //Without extension
        if(fileInfo.completeSuffix().isEmpty())
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/drag_generic.png"));
        }
        else
        {
            fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                          fileInfo.fileName(), QLatin1Literal(":/images/drag_")));
        }
    }
    else
    {
        if(hasProblem)
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/folder_error_default.png"));
        }
        else
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/folder_orange_default.png"));
        }
    }

    return fileTypeIcon;
}

//////////////////////////////////////////////////
QMap<QVariant, mega::MegaHandle> StalledIssuesBySyncFilter::mSyncIdCache = QMap<QVariant, mega::MegaHandle>();

mega::MegaHandle StalledIssuesBySyncFilter::filterByPath(const QString &path, bool cloud)
{
    QVariant key;

    //Cache in case we already checked an issue with the same path
    if(cloud)
    {
        std::unique_ptr<mega::MegaNode> remoteNode(MegaSyncApp->getMegaApi()->getNodeByPath(path.toUtf8().constData()));
        if(remoteNode)
        {
            key = remoteNode->getParentHandle();

            if(mSyncIdCache.contains(key))
            {
                return mSyncIdCache.value(key);
            }
        }
    }
    else
    {
        QFileInfo fileDir(path);
        if(!fileDir.exists())
        {
            return mega::INVALID_HANDLE;
        }

        key = fileDir.path();
        if(mSyncIdCache.contains(key))
        {
            return mSyncIdCache.value(key);
        }
    }

    std::unique_ptr<mega::MegaSyncList> syncList(MegaSyncApp->getMegaApi()->getSyncs());
    for (int i = 0; i < syncList->size(); ++i)
    {
        auto syncId(syncList->get(i)->getBackupId());
        auto syncSetting = SyncInfo::instance()->getSyncSettingByTag(syncId);

        if(syncSetting)
        {
            if(cloud)
            {
                auto remoteFolder(syncSetting->getMegaFolder());
                auto commonPath = Utilities::getCommonPath(path, remoteFolder, cloud);
                if(commonPath == remoteFolder)
                {
                    mSyncIdCache.insert(key, syncId);
                    return syncId;
                }
            }
            else
            {
                auto localFolder(syncSetting->getLocalFolder());
                auto commonPath = Utilities::getCommonPath(path, localFolder, cloud);
                if(commonPath == localFolder)
                {
                    mSyncIdCache.insert(key, syncId);
                    return syncId;
                }
            }
        }
    }


    return mega::INVALID_HANDLE;
}

bool StalledIssuesBySyncFilter::isBelow(mega::MegaHandle syncRootNode, mega::MegaHandle checkNode)
{
    if(syncRootNode == checkNode)
    {
        return true;
    }

    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(checkNode));
    if(!parentNode)
    {
        return false;
    }

    return isBelow(syncRootNode, parentNode->getParentHandle());
}

bool StalledIssuesBySyncFilter::isBelow(const QString &syncRootPath, const QString &checkPath)
{
    if(syncRootPath == checkPath)
    {
        return true;
    }

    QDir fileDir(checkPath);
    //Get parent folder
    fileDir.cdUp();
    return isBelow(syncRootPath, fileDir.path());
}

/////////////////////////////////////////////////////
/// \brief FingerprintMissingSolver::FingerprintMissingSolver
FingerprintMissingSolver::FingerprintMissingSolver()
    :mDownloader(new MegaDownloader(MegaSyncApp->getMegaApi(), nullptr))
{
}

void FingerprintMissingSolver::solveIssues(const QList<StalledIssueVariant> &pathsToSolve)
{
    QDir dir(Preferences::instance()->getTempTransfersPath());
    if (dir.exists() ||
        dir.mkpath(QString::fromUtf8(".")))
    {
        auto tempPath(Preferences::instance()->getTempTransfersPath());

        QMap<QString, std::shared_ptr<QQueue<WrappedNode*>>> nodesToDownloadByPath;
        auto appendNodeToQueue = [&](const QString& targetPath, mega::MegaNode* node)
        {
            if(!nodesToDownloadByPath.contains(targetPath))
            {
                auto queue(std::make_shared<QQueue<WrappedNode*>>());
                queue->append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
                nodesToDownloadByPath.insert(targetPath, queue);
            }
            else
            {
                auto queue = nodesToDownloadByPath.value(targetPath);
                if(queue)
                {
                    queue->append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
                }
            }
        };

        foreach(auto issue, pathsToSolve)
        {
            std::shared_ptr<DownloadTransferInfo> info(new DownloadTransferInfo());
            if(!issue.consultData()->isBeingSolvedByDownload(info))
            {
                mega::MegaNode* node(MegaSyncApp->getMegaApi()->getNodeByHandle(info->nodeHandle));

                auto localPath = issue.consultData()->consultLocalData() ?
                            issue.consultData()->consultLocalData()->getNativeFilePath() :
                            QString();
                if(!localPath.isEmpty())
                {
                    QFile localPathFile(localPath);
                    if(!localPathFile.exists())
                    {
                        QFileInfo  localFilePathInfo(localPath);
                        QString localFolderPath(localFilePathInfo.absolutePath());
                        QDir createTarget;
                        createTarget.mkdir(localFolderPath);
                        QDir localFolderPathDir(localFolderPath);
                        if(localFolderPathDir.exists())
                        {
                            appendNodeToQueue(localFolderPath, node);
                            continue;
                        }
                    }
                }

                if(node)
                {
                     appendNodeToQueue(tempPath, node);
                }
            }
        }

        foreach(auto targetFolder, nodesToDownloadByPath.keys())
        {
            std::shared_ptr<QQueue<WrappedNode*>> nodesToDownload(nodesToDownloadByPath.value(targetFolder));
            BlockingBatch downloadBatches;
            mDownloader->processDownloadQueue(nodesToDownload.get(), downloadBatches, targetFolder, targetFolder != tempPath);
            qDeleteAll(*nodesToDownload.get());
        }
    }
}
