#include "MegaDownloader.h"

#include "MegaApplication.h"
#include "Utilities.h"
#include "TransferBatch.h"

#include <QDateTime>
#include <QPointer>

#include <memory>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi* _megaApi)
    : QObject(), megaApi(_megaApi)
{
}

bool MegaDownloader::processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, BlockingBatch& downloadBatches,
                                          const QString& path, unsigned long long appDataId)
{
    mNoTransferStarted = true;
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
    TransferMetaData *data = (static_cast<MegaApplication*>(qApp))->getTransferAppData(appDataId);

    auto batch = std::shared_ptr<TransferBatch>(new TransferBatch());

    //Add now the batch in case the transfers are started from the Webclient
    //In the transfers are started from the webclient, the onTransferStart are received before
    //the while finishes, and the batch counters are not correctly updated
    downloadBatches.add(batch);

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
                update(data, appData, node, path);
            }

            currentPath = path;
        }

        // We now have all the necessary info to effectively download.
        bool transferStarted = download(wNode, currentPath, appData, batch->getCancelTokenPtr());
        if (transferStarted && (wNode->getTransferOrigin() != WrappedNode::FROM_WEBSERVER || node->isFile()))
        {
            batch->add(node->getType() != MegaNode::TYPE_FILE);
        }
        delete wNode;
    }

    if (batch->isEmpty())
    {
        downloadBatches.removeBatch();
    }

    pathMap.clear();
    return true;
}

bool MegaDownloader::download(WrappedNode* parent, QFileInfo info, QString appData, MegaCancelToken* cancelToken)
{
    QPointer<MegaDownloader> safePointer = this;

    QApplication::processEvents();
    if (!safePointer)
    {
        return false;
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
        if (mNoTransferStarted)
        {
            emit startingTransfers();
            mNoTransferStarted = false;
        }
        startDownload(parent, appData, currentPathWithSep, cancelToken);
        return true;
    }
    else
    {
        downloadForeignDir(node, currentPathWithSep, appData);
        return false;
    }
}

void MegaDownloader::startDownload(WrappedNode *parent, const QString& appData,
                                   const QString& currentPathWithSep, MegaCancelToken* cancelToken)
{
    bool startFirst = hasTransferPriority(parent->getTransferOrigin());
    QByteArray localPath = currentPathWithSep.toUtf8();
    const char* name = parent->getMegaNode()->getName();
    MegaTransferListener* listener = nullptr;
    megaApi->startDownload(parent->getMegaNode(), localPath.constData(), name, appData.toUtf8().constData(), startFirst, cancelToken, listener);
}

void MegaDownloader::downloadForeignDir(MegaNode *node, const QString& appData, const QString& currentPathWithSep)
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

void MegaDownloader::update(TransferMetaData *dataToUpdate, QString& appData, MegaNode *node, const QString &path)
{
    // Update transfer metadata according to node type.
    node->isFolder() ? dataToUpdate->totalFolders++ : dataToUpdate->totalFiles++;

    // Report that there is still a "root folder" to download/create
    appData.append(QString::fromUtf8("*"));

    // Set the metadata local path to destination path
    if (dataToUpdate->localPath.isEmpty())
    {
        dataToUpdate->localPath = QDir::toNativeSeparators(path);

        // If there is only 1 transfer, set localPath to full path
        if (dataToUpdate->totalTransfers == 1)
        {
            char *escapedName = megaApi->escapeFsIncompatible(node->getName(),
                                                              path.toStdString().c_str());
            QString nodeName = QString::fromUtf8(escapedName);
            delete [] escapedName;
            if (!dataToUpdate->localPath.endsWith(QDir::separator()))
            {
                dataToUpdate->localPath += QDir::separator();
            }
            dataToUpdate->localPath += nodeName;
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

bool MegaDownloader::createDirIfNotPresent(const QString& path)
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
