#include "MegaDownloader.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include <QDateTime>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi *megaApi, MegaApi *megaApiGuest) : QObject()
{
    this->megaApi = megaApi;
    this->megaApiGuest = megaApiGuest;
}

MegaDownloader::~MegaDownloader()
{

}

void MegaDownloader::download(MegaNode *parent, QString path, unsigned long long appDataId)
{
    return download(parent, QFileInfo(path), appDataId);
}

void MegaDownloader::processDownloadQueue(QQueue<MegaNode *> *downloadQueue, QString path, unsigned long long appDataId)
{
    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
    {
        qDeleteAll(*downloadQueue);
        downloadQueue->clear();
        return;
    }

    TransferMetaData *data = ((MegaApplication*)qApp)->getTransferAppData(appDataId);

    QString currentPath;
    while (!downloadQueue->isEmpty())
    {
        MegaNode *node = downloadQueue->dequeue();
        if (node->isForeign())
        {
            if (pathMap.contains(node->getParentHandle()))
            {
                currentPath = pathMap[node->getParentHandle()];
            }
            else
            {
                if (data)
                {
                    if (node->isFolder())
                    {
                        data->totalFolders++;
                    }
                    else
                    {
                        data->totalFiles++;
                    }
                }

                currentPath = path;
            }
        }
        else
        {
            if (data)
            {
                if (node->isFolder())
                {
                    data->totalFolders++;
                }
                else
                {
                    data->totalFiles++;
                }
            }

            currentPath = path;
        }

        download(node, currentPath, appDataId);
        delete node;
    }
    pathMap.clear();
}

void MegaDownloader::download(MegaNode *parent, QFileInfo info, unsigned long long appDataId)
{
    QApplication::processEvents();

    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());

    if (parent->getType() == MegaNode::TYPE_FILE)
    {
        if ((parent->isPublic() || parent->isForeign()) && megaApiGuest)
        {
            megaApiGuest->startDownload(parent, (currentPath + QDir::separator()).toUtf8().constData());
        }
        else
        {
            megaApi->startDownloadWithData(parent, (currentPath + QDir::separator()).toUtf8().constData(), QString::number(appDataId).toUtf8().constData());
        }
    }
    else
    {
        char *escapedName = megaApi->escapeFsIncompatible(parent->getName());
        QString nodeName = QString::fromUtf8(escapedName);
        delete [] escapedName;

        QString destPath = currentPath + QDir::separator() + nodeName;
        QDir dir(destPath);
        if (!dir.exists())
        {
#ifndef WIN32
            if (!megaApi->createLocalFolder(dir.toNativeSeparators(destPath).toUtf8().constData()))
#else
            if (!dir.mkpath(QString::fromAscii(".")))
#endif
            {
                return;
            }
        }

        TransferMetaData *data = ((MegaApplication*)qApp)->getTransferAppData(appDataId);
        if (data)
        {
            data->pendingTransfers--;
            if (data->pendingTransfers == 0)
            {
                //Transfers finished, show notification
                emit finishedTransfers(appDataId);
            }
        }

        if (!parent->isForeign())
        {
            MegaNodeList *nList = megaApi->getChildren(parent);
            for (int i = 0; i < nList->size(); i++)
            {
                MegaNode *child = nList->get(i);
                download(child, destPath, appDataId);
            }
            delete nList;
        }
        else
        {
            pathMap[parent->getHandle()] = destPath;
        }
    }
}
