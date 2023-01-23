#include "MegaDownloader.h"

#include "EventUpdater.h"
#include "LowDiskSpaceDialog.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "TransferBatch.h"

#include <QDateTime>
#include <QFileIconProvider>
#include <QPointer>

#include <memory>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi* _megaApi, std::shared_ptr<FolderTransferListener> _listener)
    : QObject(), megaApi(_megaApi), listener(_listener), mQueueData(_megaApi, pathMap)
{
    connect(&mQueueData, &DownloadQueueController::finishedAvailableSpaceCheck,
            this, &MegaDownloader::onAvailableSpaceCheckFinished);
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

    mProcessingTransferQueue = true;
    mQueueData.initialize(downloadQueue, downloadBatches, appDataId, path);
    mQueueData.startAvailableSpaceChecking();
    return true;
}

bool MegaDownloader::isQueueProcessingOngoing()
{
    return mProcessingTransferQueue;
}

bool MegaDownloader::download(WrappedNode* parent, QFileInfo info, QString appData, MegaCancelToken* cancelToken)
{
    QPointer<MegaDownloader> safePointer = this;

    if (!safePointer)
    {
        return false;
    }

    QString currentPathWithSep = createPathWithSeparator(info.absoluteFilePath());

    // Extract MEGA node from wrapped node for more readable code
    // Both parent and node should be not null at this point.
    mega::MegaNode *node {parent->getMegaNode()};
    bool isForeignDir = node->getType() != MegaNode::TYPE_FILE && node->isForeign();
    if (!isForeignDir)
    {
        bool isTransferFromApp = (parent->getTransferOrigin() == WrappedNode::FROM_APP);
        MegaCancelToken* tokenToUse = (isTransferFromApp) ? cancelToken : nullptr;
        if (mNoTransferStarted && isTransferFromApp)
        {
            emit startingTransfers();
            mNoTransferStarted = false;
        }
        startDownload(parent, appData, currentPathWithSep, tokenToUse);
        return true;
    }
    else
    {
        downloadForeignDir(node, appData, currentPathWithSep);
        return false;
    }
}

void MegaDownloader::onAvailableSpaceCheckFinished(bool isDownloadPossible)
{
    if (isDownloadPossible)
    {
        auto batch = std::shared_ptr<TransferBatch>(new TransferBatch());
        mQueueData.addTransferBatch(batch);

        TransferMetaData *metadata = (static_cast<MegaApplication*>(qApp))->getTransferAppData(mQueueData.getCurrentAppDataId());

        EventUpdater updater(mQueueData.getDownloadQueueSize());

        // Process all nodes in the download queue
        while (!mQueueData.isDownloadQueueEmpty())
        {
            WrappedNode *wNode = mQueueData.dequeueDownloadQueue();
            MegaNode *node = wNode->getMegaNode();

            QString currentPath;
            QString appData = QString::number(mQueueData.getCurrentAppDataId());

            if (node->isForeign() && pathMap.contains(node->getParentHandle()))
            {
                currentPath = pathMap[node->getParentHandle()];
            }
            else
            {
                currentPath = mQueueData.getCurrentTargetPath();

                if (metadata)
                {
                    mQueueData.update(metadata, node, currentPath);
                }

                appData.append(QLatin1Char('*'));
            }

            bool transferStarted = download(wNode, currentPath, appData, batch->getCancelTokenPtr());
            if (transferStarted && wNode->getTransferOrigin() == WrappedNode::FROM_APP)
            {
                batch->add(currentPath, QString::fromUtf8(node->getName()));
            }
            delete wNode;
            updater.update(mQueueData.getDownloadQueueSize());
        }

        if (batch->isEmpty())
        {
            mQueueData.removeBatch();
        }
    }
    else
    {
        mQueueData.clearDownloadQueue();
    }

    pathMap.clear();
    mProcessingTransferQueue = false;
}

void MegaDownloader::startDownload(WrappedNode *parent, const QString& appData,
                                   const QString& currentPathWithSep, MegaCancelToken* cancelToken)
{
    bool startFirst = hasTransferPriority(parent->getTransferOrigin());
    QByteArray localPath = currentPathWithSep.toUtf8();
    const char* name = parent->getMegaNode()->getName();
    megaApi->startDownload(parent->getMegaNode(), localPath.constData(), name, appData.toUtf8().constData(), startFirst, cancelToken, listener.get());
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

QString MegaDownloader::createPathWithSeparator(const QString &path)
{
    QString pathWithSep = QDir::toNativeSeparators(path);
    if (!pathWithSep.endsWith(QDir::separator()))
    {
        pathWithSep += QDir::separator();
    }
    return pathWithSep;
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

