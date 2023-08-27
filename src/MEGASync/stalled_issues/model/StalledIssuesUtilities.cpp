#include "StalledIssuesUtilities.h"

#include <MegaApplication.h>
#include <mega/types.h>

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

        while(ignoreDir.exists())
        {
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
