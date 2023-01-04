#include "DownloadQueueController.h"

#include "LowDiskSpaceDialog.h"
#include "MegaApplication.h"

using namespace mega;

DownloadQueueController::DownloadQueueController(MegaApi *_megaApi, const QMap<mega::MegaHandle, QString>& pathMap)
    : mMegaApi(_megaApi), mPathMap(pathMap),
      mListener(new QTMegaRequestListener(mMegaApi, this))
{
}

void DownloadQueueController::initialize(QQueue<WrappedNode *> *downloadQueue, BlockingBatch &downloadBatches,
                                   unsigned long long appDataId, const QString& path)
{
    mDownloadQueue = downloadQueue;
    mDownloadBatches = &downloadBatches;

    QString appData = QString::number(appDataId);
    TransferMetaData *metadata = (static_cast<MegaApplication*>(qApp))->getTransferAppData(appDataId);

    prepareAppDatas(appData, metadata);
    preparePaths(path, metadata);
}

void DownloadQueueController::startAvailableSpaceChecking()
{
    mTotalQueueDiskSize = 0;
    mFolderCountPendingSizeComputation = 0;
    auto accumulator = [this](qint64 partialSum, WrappedNode* currentNode)
    {
        MegaNode *node = currentNode->getMegaNode();
        if (node->getType() == MegaNode::TYPE_FILE)
        {
            partialSum += node->getSize();
        }
        else
        {
            mFolderCountPendingSizeComputation++;
            mMegaApi->getFolderInfo(node, mListener.get());
        }

        return partialSum;
    };

    auto queueStart = mDownloadQueue->cbegin();
    auto queueEnd = mDownloadQueue->cend();
    mTotalQueueDiskSize += std::accumulate(queueStart, queueEnd, 0LL, accumulator);

    if (mFolderCountPendingSizeComputation == 0)
    {
        emit finishedAvailableSpaceCheck(isDownloadPossible());
    }
}

QList<QString>::ConstIterator DownloadQueueController::getAppDatasBegin()
{
    return mAppDatas.cbegin();
}

QList<QString>::ConstIterator DownloadQueueController::getPathsBegin()
{
    return mPaths.cbegin();
}

void DownloadQueueController::addTransferBatch(std::shared_ptr<TransferBatch> batch)
{
    mDownloadBatches->add(batch);
}

void DownloadQueueController::removeBatch()
{
    mDownloadBatches->removeBatch();
}

int DownloadQueueController::getDownloadQueueSize()
{
    return mDownloadQueue->size();
}

bool DownloadQueueController::isDownloadQueueEmpty()
{
    return mDownloadQueue->empty();
}

void DownloadQueueController::clearDownloadQueue()
{
    mDownloadQueue->clear();
}

WrappedNode *DownloadQueueController::dequeueDownloadQueue()
{
    return mDownloadQueue->dequeue();
}

void DownloadQueueController::onRequestFinish(MegaApi*, MegaRequest *request, MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_FOLDER_INFO
            && e->getErrorCode() == mega::MegaError::API_OK)
    {
        auto folderInfo = request->getMegaFolderInfo();
        mTotalQueueDiskSize += folderInfo->getCurrentSize();
        --mFolderCountPendingSizeComputation;
        if (mFolderCountPendingSizeComputation <= 0)
        {
            emit finishedAvailableSpaceCheck(isDownloadPossible());
        }
    }
}

void DownloadQueueController::prepareAppDatas(const QString &appId, TransferMetaData *metadata)
{
    for (auto download : qAsConst(*mDownloadQueue))
    {
        MegaNode *node = download->getMegaNode();
        QString appData = appId;

        if (metadata)
        {
            if (!node->isForeign() || !mPathMap.contains(node->getParentHandle()))
            {
                // Report that there is still a "root folder" to download/create
                appData.append(QLatin1Char('*'));
            }
        }
        mAppDatas.push_back(appData);
    }
}

void DownloadQueueController::preparePaths(const QString &path, TransferMetaData *metadata)
{
    for (auto download : qAsConst(*mDownloadQueue))
    {
        MegaNode *node = download->getMegaNode();

        QString currentPath;
        if (node->isForeign() && mPathMap.contains(node->getParentHandle()))
        {
            currentPath = mPathMap[node->getParentHandle()];
        }
        else
        {
            if (metadata)
            {
                update(metadata, node, path);
            }

            currentPath = path;
        }
        mPaths.push_back(currentPath);
    }
}

void DownloadQueueController::update(TransferMetaData *dataToUpdate, MegaNode *node, const QString &path)
{
    // Update transfer metadata according to node type.
    node->isFolder() ? dataToUpdate->totalFolders++ : dataToUpdate->totalFiles++;

    // Set the metadata local path to destination path
    if (dataToUpdate->localPath.isEmpty())
    {
        dataToUpdate->localPath = QDir::toNativeSeparators(path);

        // If there is only 1 transfer, set localPath to full path
        if (dataToUpdate->totalTransfers == 1)
        {
            char *escapedName = mMegaApi->escapeFsIncompatible(node->getName(),
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

bool DownloadQueueController::isDownloadPossible()
{
    bool retry = true;
    bool downloadPossible = false;
    while (!downloadPossible && retry)
    {
        downloadPossible = hasEnoughSpaceForDownloads();
        if (!downloadPossible)
        {
            retry = shouldRetryWhenNotEnoughSpace();
        }
    }
    return downloadPossible;
}

bool DownloadQueueController::hasEnoughSpaceForDownloads()
{
    if (!mPaths.empty())
    {
        QStorageInfo destinationDrive(mPaths.first());
        return (mTotalQueueDiskSize < destinationDrive.bytesAvailable());
    }
    return true;
}

bool DownloadQueueController::shouldRetryWhenNotEnoughSpace()
{
    QStorageInfo destinationDrive(mPaths.first());
    QString driveName = destinationDrive.name();
    if (driveName.isEmpty())
    {
        driveName = tr("Local Disk");
    }

    LowDiskSpaceDialog dialog(mTotalQueueDiskSize, destinationDrive.bytesAvailable(),
                              destinationDrive.bytesTotal(), driveName);
    int userChoice = dialog.exec();
    return (userChoice == QDialog::Accepted);
}
