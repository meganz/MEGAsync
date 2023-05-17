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

QPair<QDateTime, QDateTime> StalledIssuesUtilities::getRemoteModificatonAndCreatedTime(mega::MegaNode *node)
{
    QPair<QDateTime, QDateTime> times;

    std::unique_ptr<mega::MegaNodeList> nodeVersions(MegaSyncApp->getMegaApi()->getVersions(node));
    if(nodeVersions->size() != 0)
    {
        auto lastVersionNode = nodeVersions->get(0);
        times.first = QDateTime::fromSecsSinceEpoch(lastVersionNode->getModificationTime());

        auto firstVersionNode = nodeVersions->get(nodeVersions->size() - 1);
        times.second = QDateTime::fromSecsSinceEpoch(firstVersionNode->getCreationTime());
    }
    else
    {
        times.first = QDateTime::fromSecsSinceEpoch(node->getModificationTime());
        times.second = QDateTime::fromSecsSinceEpoch(node->getCreationTime());
    }

    return times;
}
