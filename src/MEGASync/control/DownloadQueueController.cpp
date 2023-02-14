#include "DownloadQueueController.h"

#include "LowDiskSpaceDialog.h"
#include "MegaApplication.h"
#include "DialogOpener.h"

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
        tryDownload();
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
            tryDownload();
        }
    }
}

void DownloadQueueController::tryDownload()
{
    bool downloadPossible(hasEnoughSpaceForDownloads());
    if (!downloadPossible)
    {
        askUserForChoice();
    }
    else
    {
        emit finishedAvailableSpaceCheck(downloadPossible);
    }
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

void DownloadQueueController::askUserForChoice()
{
    QStorageInfo destinationDrive(mCurrentTargetPath);
    QString driveName = destinationDrive.name();
    if (driveName.isEmpty())
    {
        driveName = tr("Local Disk");
    }

    LowDiskSpaceDialog* dialog = new LowDiskSpaceDialog(mTotalQueueDiskSize, destinationDrive.bytesAvailable(),
                              destinationDrive.bytesTotal(), driveName);
    DialogOpener::showDialog<LowDiskSpaceDialog>(dialog, [this, dialog](){
        dialog->result() == QDialog::Accepted ? isDownloadPossible() : emit finishedAvailableSpaceCheck(false);
    });
}

const QString& DownloadQueueController::getCurrentTargetPath() const
{
    return mCurrentTargetPath;
}

unsigned long long DownloadQueueController::getCurrentAppDataId() const
{
    return mCurrentAppDataId;
}
