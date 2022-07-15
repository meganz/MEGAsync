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

MegaDownloader::~MegaDownloader()
{

}

void MegaDownloader::download(WrappedNode *parent, QString path, QString appData)
{
    return download(parent, QFileInfo(path), appData);
}

bool MegaDownloader::processDownloadQueue(QQueue<WrappedNode *> *downloadQueue, QString path, unsigned long long appDataId)
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

        // Delete the node object once the transfer has been passed over to the SDK.
        delete wNode;
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

    // If the node is a file (foreign or not), or a not foreign dir:
    if (node->getType() == MegaNode::TYPE_FILE || !node->isForeign())
    {
        // Download with priority depending on the transfer's origin
        switch (parent->getTransferOrigin()) {
            case WrappedNode::TransferOrigin::FROM_WEBSERVER :
            {
                // Downloads initiated through http server get top priority
                megaApi->startDownload(node,
                                       currentPathWithSep.toUtf8().constData(),
                                       nullptr, // no customName
                                       appData.toUtf8().constData(),
                                       true, // top priority: goes to the front of the queue
                                       nullptr, // no cancelToken
                                       nullptr); // no listener
                break;
            }
            case WrappedNode::TransferOrigin::FROM_APP :
            case WrappedNode::TransferOrigin::FROM_UNKNOWN :
            default:
            {
                // For other downloads, use normal priority call
                megaApi->startDownload(node,
                                       currentPathWithSep.toUtf8().constData(),
                                       nullptr, // no customName
                                       appData.toUtf8().constData(),
                                       false, // normal queueing
                                       nullptr, // no cancelToken
                                       nullptr); // no listener
            }
        }
    }
    // Else, the node is a foreign dir (neither a file nor a not foreign node).
    // Downloading amounts to creating the dir if it doesn't exist.
    else
    {
        // Get a c-string with the escaped name. This string will have to be deleted because
        // megaApi->escapeFsIncompatible allocates it.
        char *escapedName = megaApi->escapeFsIncompatible(node->getName(),
                                                          currentPathWithSep.toStdString().c_str());
        QString nodeName = QString::fromUtf8(escapedName);
        delete [] escapedName;

        // Build destination path and create it if it does not exist. If the creation fails, exit.
        QString destPath = currentPathWithSep + nodeName;
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
}
