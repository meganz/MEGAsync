#include "StalledIssuesUtilities.h"

#include <MegaApplication.h>
#include <mega/types.h>

#include <QFile>
#include <QDir>

StalledIssuesUtilities::StalledIssuesUtilities() : mega::MegaRequestListener(),
    mListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mRemoteHandle(0)
{}

void StalledIssuesUtilities::ignoreFile(const QString &path)
{
    connect(&mIgnoreWatcher, &QFutureWatcher<void>::finished,
            this, &StalledIssuesUtilities::onIgnoreFileFinished);

    QFuture<void> addToIgnore = QtConcurrent::run([path]()
    {
        QFileInfo tempFile(path);
        QDir ignoreDir(tempFile.path());

        while(ignoreDir.exists())
        {
            QFile ignore(ignoreDir.path() + QDir::separator() + QString::fromUtf8(".megaignore"));
            if(ignore.exists())
            {
                ignore.open(QFile::Append | QFile::Text);

                QTextStream streamIn(&ignore);
                streamIn.setCodec("UTF-8");

                streamIn << QChar((int)'\n');

                streamIn << QString::fromUtf8("-:");

                streamIn << ignoreDir.relativeFilePath(path);
                ignore.close();

                break;
            }

            if(!ignoreDir.cdUp())
            {
                break;
            }
        }
    });

    mIgnoreWatcher.setFuture(addToIgnore);
}

void StalledIssuesUtilities::onIgnoreFileFinished()
{
    emit actionFinished();
    disconnect(&mIgnoreWatcher, &QFutureWatcher<void>::finished,
               this, &StalledIssuesUtilities::onIgnoreFileFinished);
}

void StalledIssuesUtilities::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_MOVE
            || request->getType() == mega::MegaRequest::TYPE_RENAME)
    {
        if (e->getErrorCode() == mega::MegaError::API_OK)
        {
            auto handle = request->getNodeHandle();
            if(handle && handle == mRemoteHandle)
            {
                emit actionFinished();
                mRemoteHandle = 0;
            }
        }
    }
}

void StalledIssuesUtilities::removeRemoteFile(const QString& path)
{
    auto fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(path.toStdString().c_str()));
    if(fileNode)
    {
        mRemoteHandle = fileNode->getHandle();
        auto rubbishNode = MegaSyncApp->getMegaApi()->getRubbishNode();
        MegaSyncApp->getMegaApi()->moveNode(fileNode,rubbishNode, mListener.get());
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
