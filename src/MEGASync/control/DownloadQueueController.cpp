#include "DownloadQueueController.h"

#include "LowDiskSpaceDialog.h"
#include "DialogOpener.h"
#include "Platform.h"
#include "RequestListenerManager.h"

#ifdef WIN32
#include <fileapi.h>
#endif

using namespace mega;

DownloadQueueController::DownloadQueueController(MegaApi *_megaApi, const QMap<mega::MegaHandle, QString>& pathMap)
    : mMegaApi(_megaApi), mPathMap(pathMap)
{
}

void DownloadQueueController::initialize(QQueue<WrappedNode> downloadQueue,
                                         BlockingBatch* downloadBatches,
                                         unsigned long long appDataId,
                                         const QString& path)
{
    mDownloadQueue = downloadQueue;
    mDownloadBatches = downloadBatches;

    mCurrentTargetPath = path;
    mCurrentAppDataId = appDataId;
}

void DownloadQueueController::startAvailableSpaceChecking()
{
    mTotalQueueDiskSize = 0LL;
    mFolderCountPendingSizeComputation = 0;
    auto accumulator = [this](long long partialSum, const WrappedNode& currentNode)
    {
        MegaNode* node = currentNode.getMegaNode();
        if (node->getType() == MegaNode::TYPE_FILE)
        {
            partialSum += node->getSize();
        }
        else if (currentNode.getTransferOrigin() != WrappedNode::FROM_WEBSERVER)
        { // Ignore folders if the transfer comes from the webclient, because it provides
          // both all folders and all files, and not only top files/folders.
            mFolderCountPendingSizeComputation++;
            auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
            mMegaApi->getFolderInfo(node, listener.get());
        }

        return partialSum;
    };

    auto queueStart = mDownloadQueue.cbegin();
    auto queueEnd = mDownloadQueue.cend();
    mTotalQueueDiskSize += std::accumulate(queueStart, queueEnd, 0LL, accumulator);

    if (mFolderCountPendingSizeComputation == 0)
    {
        tryDownload();
    }
}

void DownloadQueueController::addTransferBatch(std::shared_ptr<TransferBatch> batch)
{
    if (mDownloadBatches)
    {
        mDownloadBatches->add(batch);
    }
}

void DownloadQueueController::removeBatch()
{
    if (mDownloadBatches)
    {
        mDownloadBatches->removeBatch();
    }
}

int DownloadQueueController::getDownloadQueueSize()
{
    return mDownloadQueue.size();
}

bool DownloadQueueController::isDownloadQueueEmpty()
{
    return mDownloadQueue.empty();
}

void DownloadQueueController::clearDownloadQueue()
{
    mDownloadQueue.clear();
}

WrappedNode DownloadQueueController::dequeueDownloadQueue()
{
    return mDownloadQueue.dequeue();
}

void DownloadQueueController::onRequestFinish(MegaRequest *request, MegaError *e)
{
    if (request->getType() != mega::MegaRequest::TYPE_FOLDER_INFO)
    {
        return;
    }

    if (e->getErrorCode() == mega::MegaError::API_OK)
    {
        auto folderInfo = request->getMegaFolderInfo();
        mTotalQueueDiskSize += folderInfo->getCurrentSize();
    }

    if (--mFolderCountPendingSizeComputation <= 0)
    {
        tryDownload();
    }
}

void DownloadQueueController::tryDownload()
{
    const bool downloadPossible = hasEnoughSpaceForDownloads();
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
        mCachedDriveData = getDriveSpaceDataFromQt();
        if (!mCachedDriveData.isAvailable())
        {
            mCachedDriveData = Platform::getInstance()->getDriveData(mCurrentTargetPath);
        }
        return (!mCachedDriveData.mIsReady || mTotalQueueDiskSize < mCachedDriveData.mAvailableSpace);
    }
    return true;
}

void DownloadQueueController::askUserForChoice()
{
    QStorageInfo destinationDrive(mCurrentTargetPath);

    const QString driveName = getDriveName(destinationDrive);

    LowDiskSpaceDialog* dialog = new LowDiskSpaceDialog(mTotalQueueDiskSize,
                                                        mCachedDriveData.mAvailableSpace,
                                                        mCachedDriveData.mTotalSpace,
                                                        driveName);
    DialogOpener::showDialog<LowDiskSpaceDialog>(dialog, [this, dialog](){
        dialog->result() == QDialog::Accepted ? tryDownload() : emit finishedAvailableSpaceCheck(false);
    });
}

QString DownloadQueueController::getDriveName(const QStorageInfo& driveInfo) const
{
    const QString driveName = driveInfo.name();
    return driveName.isEmpty() ? getDefaultDriveName() : driveName;
}

QString DownloadQueueController::getDefaultDriveName() const
{
#ifdef WIN32
    UINT driveType = GetDriveTypeA(mCurrentTargetPath.toUtf8().constData());
    if (driveType == DRIVE_REMOVABLE || driveType == DRIVE_CDROM)
    {
        return tr("Removable drive");
    }
    else if (driveType == DRIVE_REMOTE || driveType == DRIVE_UNKNOWN || driveType == DRIVE_NO_ROOT_DIR)
    {
        return tr("Shared drive");
    }
#endif
    return tr("Local drive");
}

DriveSpaceData DownloadQueueController::getDriveSpaceDataFromQt()
{
    DriveSpaceData data;

    QStorageInfo destinationDrive(mCurrentTargetPath);
    data.mIsReady = destinationDrive.isReady();
    data.mAvailableSpace = destinationDrive.bytesAvailable();
    data.mTotalSpace = destinationDrive.bytesTotal();

    return data;
}

const QString& DownloadQueueController::getCurrentTargetPath() const
{
    return mCurrentTargetPath;
}

unsigned long long DownloadQueueController::getCurrentAppDataId() const
{
    return mCurrentAppDataId;
}
