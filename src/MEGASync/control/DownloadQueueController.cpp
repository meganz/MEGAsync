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

    mCurrentTargetPath = path;
    mCurrentAppDataId = appDataId;
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
    if (!mCurrentTargetPath.isEmpty())
    {
        QStorageInfo destinationDrive(mCurrentTargetPath);
        return (mTotalQueueDiskSize < destinationDrive.bytesAvailable());
    }
    return true;
}

bool DownloadQueueController::shouldRetryWhenNotEnoughSpace()
{
    QStorageInfo destinationDrive(mCurrentTargetPath);
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

const QString& DownloadQueueController::getCurrentTargetPath() const
{
    return mCurrentTargetPath;
}

unsigned long long DownloadQueueController::getCurrentAppDataId() const
{
    return mCurrentAppDataId;
}
