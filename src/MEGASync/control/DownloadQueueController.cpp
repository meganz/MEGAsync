#include "DownloadQueueController.h"

#include "LowDiskSpaceDialog.h"
#include "MegaApplication.h"
#include "DialogOpener.h"
#include "platform/Platform.h"

#ifdef WIN32
#include <fileapi.h>
#endif

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
        else if (currentNode->getTransferOrigin() != WrappedNode::FROM_WEBSERVER)
        { // Ignore folders if the transfer comes from the webclient, because it provides
          // both all folders and all files, and not only top files/folders.
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
    if (request->getType() == mega::MegaRequest::TYPE_FOLDER_INFO)
    {
        if(e->getErrorCode() == mega::MegaError::API_OK)
        {
            auto folderInfo = request->getMegaFolderInfo();
            mTotalQueueDiskSize += folderInfo->getCurrentSize();
        }

        --mFolderCountPendingSizeComputation;
        if (mFolderCountPendingSizeComputation <= 0)
        {
            tryDownload();
        }
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

    const DriveDisplayData driveDisplayData = getDriveDisplayData(destinationDrive);


    LowDiskSpaceDialog* dialog = new LowDiskSpaceDialog(mTotalQueueDiskSize, mCachedDriveData.mAvailableSpace,
                              mCachedDriveData.mTotalSpace, driveDisplayData);
    DialogOpener::showDialog<LowDiskSpaceDialog>(dialog, [this, dialog](){
        dialog->result() == QDialog::Accepted ? tryDownload() : emit finishedAvailableSpaceCheck(false);
    });
}

DriveDisplayData DownloadQueueController::getDriveDisplayData(const QStorageInfo &driveInfo) const
{
    DriveDisplayData data;
    data.name = driveInfo.name();
    if (data.name.isEmpty())
    {
        data.name = getDefaultDriveName();
    }
    data.icon = getDriveIcon();
    return data;
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

QString DownloadQueueController::getDriveIcon() const
{
    return QString::fromLatin1(":/images/drive-low-space.svg");
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
