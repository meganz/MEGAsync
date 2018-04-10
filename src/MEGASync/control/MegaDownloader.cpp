#include "MegaDownloader.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include <QDateTime>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi *megaApi) : QObject()
{
    this->megaApi = megaApi;
}

MegaDownloader::~MegaDownloader()
{

}

void MegaDownloader::download(MegaNode *parent, QString path, unsigned long long appDataId)
{
    return download(parent, QFileInfo(path), appDataId);
}

bool MegaDownloader::processDownloadQueue(QQueue<MegaNode *> *downloadQueue, QString path, unsigned long long appDataId)
{
    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
    {
        qDeleteAll(*downloadQueue);
        downloadQueue->clear();
        return false;
    }

    TransferMetaData *data = ((MegaApplication*)qApp)->getTransferAppData(appDataId);

    QString currentPath;
    while (!downloadQueue->isEmpty())
    {
        MegaNode *node = downloadQueue->dequeue();
        if (node->isForeign() && pathMap.contains(node->getParentHandle()))
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

                if (data->localPath.isEmpty())
                {
                    data->localPath = QDir::toNativeSeparators(path);
                    if (data->totalTransfers == 1)
                    {
                        char *escapedName = megaApi->escapeFsIncompatible(node->getName());
                        QString nodeName = QString::fromUtf8(escapedName);
                        delete [] escapedName;
                        data->localPath += QDir::separator() + nodeName;
                    }
                }
            }

            currentPath = path;
        }

        download(node, currentPath, appDataId);
        delete node;
    }
    pathMap.clear();
    return true;
}

void MegaDownloader::download(MegaNode *parent, QFileInfo info, unsigned long long appDataId)
{
    QApplication::processEvents();

    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());

    if (parent->getType() == MegaNode::TYPE_FILE)
    {
        megaApi->startDownloadWithData(parent, (currentPath + QDir::separator()).toUtf8().constData(), QString::number(appDataId).toUtf8().constData());
    }
    else
    {
        if (!parent->isForeign())
        {
            megaApi->startDownloadWithData(parent, (currentPath + QDir::separator()).toUtf8().constData(), QString::number(appDataId).toUtf8().constData());
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

            pathMap[parent->getHandle()] = destPath;
        }
    }
}
