#include "MegaDownloader.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include <QDateTime>
#include <QPointer>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi *megaApi) : QObject()
{
    this->megaApi = megaApi;
}

void MegaDownloader::download(WrappedNode *parent, QString path, QString appData)
{
    download(parent, QFileInfo(path), appData);
}

bool MegaDownloader::processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, std::vector<WrappedNode*>* ongoingDownloads, QString path, unsigned long long appDataId)
{
    // If the destination path doesn't exist and we can't create it,
    // empty queue and abort transfer.
    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QString::fromUtf8(".")))
    {
        qDeleteAll(*downloadQueue);
        downloadQueue->clear();
        return false;
    }

    // Get transfer's metadata
    TransferMetaData *data = ((MegaApplication*)qApp)->getTransferAppData(appDataId);

    // Process all nodes in the download queue
    while (!downloadQueue->isEmpty())
    {
        static QString currentPath;

        QString appData = QString::number(appDataId);
        WrappedNode *wNode = downloadQueue->dequeue();
        MegaNode *node = wNode->getMegaNode();

        // Get path from pathMap or build it if necessary
        if (node->isForeign() && pathMap.contains(node->getParentHandle()))
        {
            currentPath = pathMap[node->getParentHandle()];
        }
        else
        {
            if (data)
            {
                // Update transfer metadata according to node type.
                if (node->isFolder())
                {
                    data->totalFolders++;
                }
                else
                {
                    data->totalFiles++;
                }

                // Report that there is still a "root folder" to download/create
                appData.append(QString::fromUtf8("*"));

                // Set the metadata local path to destination path
                if (data->localPath.isEmpty())
                {
                    data->localPath = QDir::toNativeSeparators(path);

                    // If there is only 1 transfer, set localPath to full path
                    if (data->totalTransfers == 1)
                    {
                        char *escapedName = megaApi->escapeFsIncompatible(node->getName(),
                                                                          path.toStdString().c_str());
                        QString nodeName = QString::fromUtf8(escapedName);
                        delete [] escapedName;
                        if (!data->localPath.endsWith(QDir::separator()))
                        {
                            data->localPath += QDir::separator();
                        }
                        data->localPath += nodeName;
                    }
                }
            }

            currentPath = path;
        }

        // We now have all the necessary info to effectively download.
        download(wNode, currentPath, appData);
        ongoingDownloads->push_back(wNode);
    }
    pathMap.clear();
    return true;
}

void MegaDownloader::download(WrappedNode *parent, QFileInfo info, QString appData)
{
    QPointer<MegaDownloader> safePointer = this;

    QApplication::processEvents();
    if (!safePointer)
    {
        return;
    }

    QString currentPathWithSep = QDir::toNativeSeparators(info.absoluteFilePath());
    if (!currentPathWithSep.endsWith(QDir::separator()))
    {
        currentPathWithSep += QDir::separator();
    }

    // Extract MEGA node from wrapped node for more readable code
    // Both parent and node should be not null at this point.
    mega::MegaNode *node {parent->getMegaNode()};
    bool isForeignDir = node->getType() != MegaNode::TYPE_FILE && node->isForeign();
    if (!isForeignDir)
    {
        startDownload(parent, currentPathWithSep, appData);
    }
    else
    {
        downloadForeignDir(node, currentPathWithSep, appData);
    }
}

void MegaDownloader::startDownload(WrappedNode *parent, QString appData, QString currentPathWithSep)
{
    bool startFirst = hasTransferPriority(parent->getTransferOrigin());
    const char* localPath = currentPathWithSep.toUtf8().constData();
    const char* name = parent->getMegaNode()->getName();
    MegaCancelToken* cancelToken = MegaCancelToken::createInstance();
    MegaTransferListener* listener = nullptr;
    megaApi->startDownload(parent->getMegaNode(), localPath, name, appData.toUtf8().constData(), startFirst, cancelToken, listener);
    parent->setCancelToken(cancelToken);
}

void MegaDownloader::downloadForeignDir(MegaNode *node, QString appData, QString currentPathWithSep)
{
    // Downloading amounts to creating the dir if it doesn't exist.

    QString destPath = buildEscapedPath(node->getName(), currentPathWithSep);
    if (!createDirIfNotPresent(destPath))
    {
        return;
    }

    // Once the folder has been checked for existence/created with success:
    // - check if this was A "root folder" for the transfer (if yes, update
    //     transfer metadata)
    // - check if this was the last pending transfer. If yes, emit notification.
    QByteArray appDataArray = appData.toUtf8();
    char *endptr;
    unsigned long long notificationId = strtoull(appDataArray.constData(), &endptr, 10);
    TransferMetaData *data = ((MegaApplication*)qApp)->getTransferAppData(notificationId);
    if (data)
    {
        // Thus, if there is a '*', this was a "root folder", and we successfully transfered it.
        if (*endptr == '*')
        {
            data->transfersFolderOK++;
        }

        // Update pending transfers in metadata, and notify if this was the last.
        data->pendingTransfers--;
        if (data->pendingTransfers == 0)
        {
            //Transfers finished, show notification
            emit finishedTransfers(notificationId);
        }
    }

    // Add path to pathMap
    pathMap[node->getHandle()] = destPath;
}

bool MegaDownloader::hasTransferPriority(const WrappedNode::TransferOrigin &origin)
{
    switch (origin)
    {
        case WrappedNode::TransferOrigin::FROM_WEBSERVER :
        {
            // Downloads initiated through http server get top priority
            return true;
        }
        case WrappedNode::TransferOrigin::FROM_APP :
        case WrappedNode::TransferOrigin::FROM_UNKNOWN :
        default:
        {
            // For other downloads, use normal priority call
            return false;
        }
    }
}

QString MegaDownloader::buildEscapedPath(const char *nodeName, QString currentPathWithSep)
{
    // Get a c-string with the escaped name. This string will have to be deleted because
    // megaApi->escapeFsIncompatible allocates it.
    char *escapedName = megaApi->escapeFsIncompatible(nodeName, currentPathWithSep.toStdString().c_str());
    QString escapedNameStr = QString::fromUtf8(escapedName);
    delete [] escapedName;
    return currentPathWithSep + escapedNameStr;
}

bool MegaDownloader::createDirIfNotPresent(QString path)
{
    QDir dir(path);
    if (!dir.exists())
    {
#ifndef WIN32
        if (!megaApi->createLocalFolder(dir.toNativeSeparators(path).toUtf8().constData()))
#else
        if (!dir.mkpath(QString::fromAscii(".")))
#endif
        {
            return false;
        }
    }
    return true;
}
