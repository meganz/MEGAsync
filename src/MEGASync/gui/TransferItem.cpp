#include "TransferItem.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "TransfersModel.h"

using namespace mega;

const TransferData::TransferStates TransferData::STATE_MASK = TransferData::TransferStates (
        TransferData::TransferState::TRANSFER_QUEUED |
        TransferData::TransferState::TRANSFER_ACTIVE |
        TransferData::TransferState::TRANSFER_PAUSED |
        TransferData::TransferState::TRANSFER_RETRYING |
        TransferData::TransferState::TRANSFER_COMPLETING |
        TransferData::TransferState::TRANSFER_COMPLETED |
        TransferData::TransferState::TRANSFER_CANCELLED |
        TransferData::TransferState::TRANSFER_FAILED);

const TransferData::TransferTypes TransferData::TYPE_MASK = TransferData::TransferTypes (
        TransferData::TransferType::TRANSFER_LTCPDOWNLOAD |
        TransferData::TransferType::TRANSFER_UPLOAD |
        TransferData::TransferType::TRANSFER_DOWNLOAD);

const TransferData::TransferStates TransferData::FINISHED_STATES_MASK = TransferData::TransferStates (
        TransferData::TransferState::TRANSFER_COMPLETED
        | TransferData::TransferState::TRANSFER_CANCELLED
        | TransferData::TransferState::TRANSFER_FAILED);

const TransferData::TransferStates TransferData::PAUSABLE_STATES_MASK = TransferData::TransferStates (
        TransferData::TransferState::TRANSFER_QUEUED
        | TransferData::TransferState::TRANSFER_ACTIVE
        | TransferData::TransferState::TRANSFER_RETRYING);

const TransferData::TransferStates TransferData::CANCELABLE_STATES_MASK = TransferData::TransferStates (
        TransferData::TransferState::TRANSFER_QUEUED
        | TransferData::TransferState::TRANSFER_ACTIVE
        | TransferData::TransferState::TRANSFER_PAUSED
        | TransferData::TransferState::TRANSFER_RETRYING);

const TransferData::TransferStates TransferData::ACTIVE_STATES_MASK = TransferData::TransferStates (
    TransferData::TransferState::TRANSFER_ACTIVE |
    TransferData::TransferState::TRANSFER_PAUSED |
    TransferData::TransferState::TRANSFER_COMPLETING |
    TransferData::TransferState::TRANSFER_QUEUED |
    TransferData::TransferState::TRANSFER_RETRYING
);

void TransferData::update(mega::MegaTransfer* transfer)
{
    if(transfer)
    {   
        mTag = transfer->getTag();

        mPath = QString::fromUtf8(transfer->getPath());

        mFilename = QString::fromUtf8(transfer->getFileName());
        mType = static_cast<TransferData::TransferType>(1 << transfer->getType());
        if (transfer->isSyncTransfer())
        {
            mType |= TransferData::TRANSFER_SYNC;
        }

        mFileType = Utilities::getFileType(mFilename, QString());

        mState = static_cast<TransferData::TransferState>(1 << transfer->getState());

        if(mState & TransferData::FINISHED_STATES_MASK)
        {
            mFinishedTime = QDateTime::currentSecsSinceEpoch();
            mFinishedTime += (transfer->getUpdateTime() - transfer->getStartTime()) / 10;
        }

        mPriority = transfer->getPriority();

        mTransferredBytes = static_cast<unsigned long long>(transfer->getTransferredBytes());
        mTotalSize = static_cast<unsigned long long>(transfer->getTotalBytes());
        unsigned long long remBytes = mTotalSize - mTransferredBytes;

        long long httpSpeed = static_cast<unsigned long long>(MegaSyncApp->getMegaApi()->getCurrentSpeed(transfer->getType()));
        mSpeed = std::min(transfer->getSpeed(), httpSpeed);

        TransferRemainingTime rem(mSpeed, remBytes);
        mRemainingTime = rem.calculateRemainingTimeSeconds(mSpeed, remBytes).count();

        mNotificationNumber = transfer->getNotificationNumber();
        mMeanSpeed = static_cast<unsigned long long>(transfer->getMeanSpeed());
        mErrorCode = MegaError::API_OK;
        mErrorValue = 0LL;
        mTemporaryError = false;
        mFailedTransfer = nullptr;

        auto megaError (transfer->getLastErrorExtended());
        if (megaError)
        {
            mErrorCode = megaError->getErrorCode();
            mErrorValue = megaError->getValue();
        }

        mParentHandle = transfer->getParentHandle();
        mNodeHandle = transfer->getNodeHandle();
    }
}

void TransferData::removeFailedTransfer()
{
    mFailedTransfer.reset();
}

uint64_t TransferData::getFinishedTime() const
{
    QDateTime now = QDateTime::currentDateTime();
    auto result = (now.toMSecsSinceEpoch() - (mFinishedTime*1000))/1000;
    return result;
}

QString TransferData::path() const
{
    QString localPath = mPath;
    #ifdef WIN32
    if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
    {
        localPath = localPath.mid(4);
    }
    #endif
    return localPath;
}

bool TransferData::isPublicNode() const
{
    auto result(false);

    //TODO publicNode
    if(mNodeHandle)
    {
        MegaNode *ownNode = ((MegaApplication*)qApp)->getMegaApi()->getNodeByHandle(mNodeHandle);
        if (ownNode)
        {
           auto access = ((MegaApplication*)qApp)->getMegaApi()->getAccess(ownNode);
           if (access == MegaShare::ACCESS_OWNER)
           {
               result = true;
           }
           delete ownNode;
        }
    }

    return result;
}

bool TransferData::isCancelable() const
{
    return (mState & CANCELABLE_STATES_MASK) && !isSyncTransfer();
}

bool TransferData::isUpload() const
{
    return mType & TRANSFER_UPLOAD;
}

bool TransferData::isSyncTransfer() const
{
    return mType & TRANSFER_SYNC;
}

bool TransferData::isDownload() const
{
    return mType > TRANSFER_UPLOAD;
}

bool TransferData::isFinished() const
{
    return mState & FINISHED_STATES_MASK;
}
