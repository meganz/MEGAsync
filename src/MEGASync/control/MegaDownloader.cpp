#include "MegaDownloader.h"

#include "EventUpdater.h"
#include "LowDiskSpaceDialog.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "TransferBatch.h"
#include "transfers/model/TransferMetaData.h"

#include <QDateTime>
#include <QFileIconProvider>
#include <QPointer>

#include <memory>

using namespace mega;

MegaDownloader::MegaDownloader(MegaApi* _megaApi, std::shared_ptr<FolderTransferListener> _listener)
    : QObject(), megaApi(_megaApi), mFolderTransferListener(_listener), mQueueData(_megaApi, pathMap)
{
    connect(&mQueueData, &DownloadQueueController::finishedAvailableSpaceCheck,
            this, &MegaDownloader::onAvailableSpaceCheckFinished, Qt::DirectConnection);
}

bool MegaDownloader::processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, BlockingBatch& downloadBatches,
                                          const QString& path, bool createAppDataId)
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

    unsigned long long appDataId(0);

    if(createAppDataId)
    {
        auto data = TransferMetaDataContainer::createTransferMetaData<DownloadTransferMetaData>(path);
        appDataId = data->getAppId();
    }

    mQueueData.initialize(downloadQueue, downloadBatches, appDataId, path);
    mQueueData.startAvailableSpaceChecking();
    return true;
}

void MegaDownloader::download(WrappedNode* parent, QFileInfo info, const std::shared_ptr<DownloadTransferMetaData> &data, MegaCancelToken* cancelToken)
{
    QPointer<MegaDownloader> safePointer = this;

    if (!safePointer)
    {
        return;
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
        startDownload(parent, data ? QString::number(data->getAppId()) : QString::number(0), currentPathWithSep, tokenToUse);
    }
    else
    {
        downloadForeignDir(node, data, currentPathWithSep);
    }
}

void MegaDownloader::onAvailableSpaceCheckFinished(bool isDownloadPossible)
{
    if (isDownloadPossible)
    {
        std::shared_ptr<TransferBatch> batch(nullptr);
        std::shared_ptr<DownloadTransferMetaData> appData(nullptr);

        if(mQueueData.getCurrentAppDataId() > 0)
        {
            appData = TransferMetaDataContainer::getAppDataById<DownloadTransferMetaData>(mQueueData.getCurrentAppDataId());
            if(appData)
            {
                appData->setInitialTransfers(mQueueData.getDownloadQueueSize());
                batch.reset(new TransferBatch(appData->getAppId()));
                mQueueData.addTransferBatch(batch);
            }
        }

        EventUpdater updater(mQueueData.getDownloadQueueSize());

        // Process all nodes in the download queue
        while (!mQueueData.isDownloadQueueEmpty())
        {
            WrappedNode *wNode = mQueueData.dequeueDownloadQueue();
            MegaNode *node = wNode->getMegaNode();

            QString currentPath;

            if (node->isForeign() && pathMap.contains(node->getParentHandle()))
            {
                currentPath = pathMap[node->getParentHandle()];
            }
            else
            {
                currentPath = mQueueData.getCurrentTargetPath();
            }

            download(wNode, currentPath, appData, batch ? batch->getCancelTokenPtr() : nullptr);
            delete wNode;
            updater.update(mQueueData.getDownloadQueueSize());
        }

        if (batch && batch->isEmpty())
        {
            mQueueData.removeBatch();
        }
    }
    else
    {
        if(mQueueData.getCurrentAppDataId() > 0)
        {
            TransferMetaDataContainer::removeAppData(mQueueData.getCurrentAppDataId());
        }

        mQueueData.clearDownloadQueue();
    }

    pathMap.clear();
}

void MegaDownloader::startDownload(WrappedNode *parent, const QString& appData,
                                   const QString& currentPathWithSep, MegaCancelToken* cancelToken)
{
    bool startFirst = hasTransferPriority(parent->getTransferOrigin());
    QByteArray localPath = currentPathWithSep.toUtf8();
    const char* name = parent->getMegaNode()->getName();
    megaApi->startDownload(parent->getMegaNode(), localPath.constData(), name,
                           appData.toUtf8().constData(), startFirst, cancelToken,
                           MegaTransfer::COLLISION_CHECK_FINGERPRINT,
                           MegaTransfer::COLLISION_RESOLUTION_NEW_WITH_N,
                           mFolderTransferListener.get());
}

void MegaDownloader::downloadForeignDir(MegaNode *node, const std::shared_ptr<DownloadTransferMetaData> &data, const QString& currentPathWithSep)
{
    // Downloading amounts to creating the dir if it doesn't exist.

    QString destPath = buildEscapedPath(node->getName(), currentPathWithSep);
    if (!createDirIfNotPresent(destPath))
    {
        return;
    }

    // Once the folder has been checked for existence/created with success:
    // - check if this was A "root folder" for the transfer with updateForeignDir (if yes, update
    //     transfer metadata)
    if (data)
    {
        data->updateForeignDir(node->getParentHandle());
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

