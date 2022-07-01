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
    auto megaApi = MegaSyncApp->getMegaApi();
    if(transfer && megaApi)
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

        mPreviousState = mState;
        mState = static_cast<TransferData::TransferState>(1 << transfer->getState());
        mNotificationNumber = transfer->getNotificationNumber();
        mErrorCode = MegaError::API_OK;
        mErrorValue = 0LL;
        mTemporaryError = false;
        mFailedTransfer = nullptr;
        mPriority = transfer->getPriority();
        mTransferredBytes = static_cast<unsigned long long>(transfer->getTransferredBytes());
        mTotalSize = static_cast<unsigned long long>(transfer->getTotalBytes());
        mIgnorePauseQueueState = false;

        if(mState & TransferData::FINISHED_STATES_MASK)
        {
            mFinishedTime = QDateTime::currentSecsSinceEpoch();
            mFinishedTime += (transfer->getUpdateTime() - transfer->getStartTime());

            mSpeed = transfer->getMeanSpeed() != 0 ?  static_cast<unsigned long long>(transfer->getMeanSpeed()) : mTotalSize;
        }
        else
        {
            if(mState & TransferData::TRANSFER_COMPLETING)
            {
                mSpeed = 0;
            }
            else
            {
                long long httpSpeed = static_cast<unsigned long long>(megaApi->getCurrentSpeed(transfer->getType()));
                mSpeed = std::min(transfer->getSpeed(), httpSpeed);
            }

            mMeanSpeed = 0;
            mFinishedTime = 0;
        }

        if(mTotalSize > mTransferredBytes)
        {
            unsigned long long remBytes = mTotalSize - mTransferredBytes;
            TransferRemainingTime rem(mSpeed, remBytes);
            mRemainingTime = rem.calculateRemainingTimeSeconds(mSpeed, remBytes).count();
        }
        else
        {
            mRemainingTime = 0;
        }

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

bool TransferData::hasChanged(QExplicitlySharedDataPointer<TransferData> data)
{
    bool result = true;

    if(mState == data->mState && mPriority == data->mPriority)
    {
        if(mTransferredBytes == data->mTransferredBytes &&
                data->mState != TransferData::TransferState::TRANSFER_COMPLETING)
        {
            result = false;
        }
    }

    return result;
}

bool TransferData::stateHasChanged()
{
    return mPreviousState != mState;
}

void TransferData::removeFailedTransfer()
{
    mFailedTransfer.reset();
}

void TransferData::setPauseResume(bool isPaused)
{
    if(isPaused)
    {
        setState(TransferData::TRANSFER_PAUSED);
    }
    else
    {
        setState(TransferData::TRANSFER_QUEUED);
    }

    mIgnorePauseQueueState = true;
}

bool TransferData::checkState(const TransferState& state)
{
    if(mIgnorePauseQueueState)
    {
        if(mState == state)
        {
            mIgnorePauseQueueState = false;
        }
        else if(state != TransferData::TRANSFER_PAUSED && state != TransferData::TRANSFER_QUEUED)
        {
            mIgnorePauseQueueState = false;
        }
    }

    return mIgnorePauseQueueState;
}

void TransferData::resetIgnoreUpdateUntilSameState()
{
    mIgnorePauseQueueState = false;
}

void TransferData::setState(const TransferState &state)
{
    mPreviousState = mState;
    mState = state;
}

void TransferData::setPreviousState(const TransferState &state)
{
    mPreviousState = state;
}

TransferData::TransferState TransferData::getState() const
{
    return mState;
}

int64_t TransferData::getRawFinishedTime() const
{
    return mFinishedTime;
}

int64_t TransferData::getFinishedTime() const
{
    QDateTime now = QDateTime::currentDateTime();
    int64_t  result(now.toSecsSinceEpoch() - mFinishedTime);
    return result;
}

QString TransferData::getFormattedFinishedTime() const
{
    return QDateTime::fromTime_t(static_cast<uint>(mFinishedTime)).toLocalTime().toString(QString::fromLatin1("hh:mm"));
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

    if(mNodeHandle)
    {
        std::unique_ptr<MegaNode> ownNode(MegaSyncApp->getMegaApi()->getNodeByHandle(mNodeHandle));

        if (ownNode && MegaSyncApp->getMegaApi()->getAccess(ownNode.get()) == MegaShare::ACCESS_OWNER)
        {
            result = true;
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

bool TransferData::isActive() const
{
    return mState & ACTIVE_STATES_MASK;
}

bool TransferData::isPaused() const
{
    return mState & TRANSFER_PAUSED;
}

bool TransferData::isProcessing() const
{
    return mState & (TRANSFER_ACTIVE | TRANSFER_COMPLETING);
}

bool TransferData::isCompleted() const
{
    return mState & TRANSFER_COMPLETED;
}

bool TransferData::isCompleting() const
{
    return mState & TRANSFER_COMPLETING;
}

bool TransferData::isFailed() const
{
    return mState & TRANSFER_FAILED;
}

bool TransferData::isFinished() const
{
    return mState & FINISHED_STATES_MASK;
}
