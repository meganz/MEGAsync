#include "StalledIssuesUtilities.h"

#include <MegaApplication.h>
#include <mega/types.h>

#include <QFile>
#include <QDir>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief StalledIssuesSyncDebrisUtilities::moveToSyncDebris
/// \param handles
/// This method is run synchronously. So, it waits for the SDK to create the folders, and only then we move the files
/// As the SDK createFolder is async, we need to use QEventLoop to stop the thread (which is not the main thread, so UI will not be frozen)
/// When the createFolder returns we continue or stop the eventloop.
/// When all files are moved, we stop the eventloop and we continue
bool StalledIssuesSyncDebrisUtilities::moveToSyncDebris(const QList<mega::MegaHandle> &handles)
{
    mHandles = handles;

    auto moveToDateFolder = [this](std::unique_ptr<mega::MegaNode> parentNode, const QString& duplicatedSyncFolderPath)
    {
        auto moveLambda = [this](std::unique_ptr<mega::MegaNode> rubbishNode){
            foreach(auto handle, mHandles)
            {
                std::unique_ptr<mega::MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
                if(nodeToMove)
                {
                    MegaSyncApp->getMegaApi()->moveNode(nodeToMove.get(),rubbishNode.get());
                }
            }

            mResult = true;
            mEventLoop.quit();
        };

        QString dateFolder(QDate::currentDate().toString(Qt::DateFormat::ISODate));
        QString dateFolderPath(QString::fromLatin1("%1/%2").arg(duplicatedSyncFolderPath, dateFolder));
        std::unique_ptr<mega::MegaNode> duplicatedRubbishDateNode(MegaSyncApp->getMegaApi()->getNodeByPath(dateFolderPath.toUtf8().constData()));
        if(!duplicatedRubbishDateNode)
        {
            MegaSyncApp->getMegaApi()->createFolder(dateFolder.toUtf8().constData(), parentNode.get(), new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                      [this, moveLambda]
                                                      (const mega::MegaRequest& request,const mega::MegaError& e)
            {
                if (e.getErrorCode() == mega::MegaError::API_OK)
                {
                    std::unique_ptr<mega::MegaNode> newFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(request.getNodeHandle()));
                    if(newFolder)
                    {
                        moveLambda(std::move(newFolder));
                        return;
                    }
                }

                mEventLoop.quit();
            }));

            if(!mEventLoop.isRunning())
            {
                //In order to execute in synchronously
                mEventLoop.exec();
            }
        }
        else
        {
            moveLambda(std::move(duplicatedRubbishDateNode));
        }
    };

    auto binNode = MegaSyncApp->getRubbishNode();

    QString duplicatedFolder(QLatin1String("SyncDuplicated"));
    QString fullDuplicatedFolderPath(QString::fromLatin1("//bin/%1").arg(duplicatedFolder));

    std::unique_ptr<mega::MegaNode> duplicatedRubbishNode(MegaSyncApp->getMegaApi()->getNodeByPath(fullDuplicatedFolderPath.toUtf8().constData()));
    if(!duplicatedRubbishNode)
    {
        MegaSyncApp->getMegaApi()->createFolder(duplicatedFolder.toUtf8().constData(), binNode.get(), new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                                                                [this, fullDuplicatedFolderPath, moveToDateFolder]
                                                                                                                                (const mega::MegaRequest& request,const mega::MegaError& e)
        {
            if (e.getErrorCode() == mega::MegaError::API_OK)
            {
                std::unique_ptr<mega::MegaNode> newFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(request.getNodeHandle()));
                if(newFolder)
                {
                    moveToDateFolder(std::move(newFolder),fullDuplicatedFolderPath);
                    return;
                }
            }

            mEventLoop.quit();
        }));

        //In order to execute in synchronously
        mEventLoop.exec();
    }
    else
    {
        moveToDateFolder(std::move(duplicatedRubbishNode), fullDuplicatedFolderPath);
    }

    return mResult;
}

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
        QPointer<StalledIssuesUtilities> currentThis(this);
        MegaSyncApp->getMegaApi()->moveNode(node,rubbishNode,
                                            new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(), [=](const mega::MegaRequest& request,const mega::MegaError& e){
            if(currentThis)
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

    return getFileIcon(isFile, fileInfo, hasProblem);
}

QIcon StalledIssuesUtilities::getRemoteFileIcon(mega::MegaNode *node, const QFileInfo& fileInfo, bool hasProblem)
{
    if(node)
    {
        return getFileIcon(node->isFile(), fileInfo, hasProblem);
    }
    else
    {
        return Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/help-circle.png"));
    }
}

QIcon StalledIssuesUtilities::getFileIcon(bool isFile, const QFileInfo& fileInfo, bool hasProblem)
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
