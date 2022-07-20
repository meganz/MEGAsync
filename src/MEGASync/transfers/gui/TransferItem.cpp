#include "TransferItem.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "TransfersModel.h"

using namespace mega;

const unsigned long long ACTIVE_PRIORITY_OFFSET = 100000000000000;
const unsigned long long COMPLETED_PRIORITY_OFFSET = 200000000000000;

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

const TransferData::TransferStates TransferData::PROCESSING_STATES_MASK = TransferData::TransferStates (
            TransferData::TransferState::TRANSFER_ACTIVE |
            TransferData::TransferState::TRANSFER_COMPLETING |
            TransferData::TransferState::TRANSFER_QUEUED |
            TransferData::TransferState::TRANSFER_RETRYING);

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

        //Update priority before setState as the setState changes the priority
        mPriority = transfer->getPriority();

        setState(static_cast<TransferData::TransferState>(1 << transfer->getState()));
        mNotificationNumber = transfer->getNotificationNumber();
        mErrorCode = MegaError::API_OK;
        mErrorValue = 0LL;
        mTemporaryError = false;
        mFailedTransfer = nullptr;
        mTransferredBytes = static_cast<unsigned long long>(transfer->getTransferredBytes());
        mTotalSize = static_cast<unsigned long long>(transfer->getTotalBytes());
        mIgnorePauseQueueState = false;

        if(mState & TransferData::FINISHED_STATES_MASK)
        {
            mFinishedTime = transfer->getUpdateTime();
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

bool TransferData::ignoreUpdate(const TransferState& state)
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
    if(mState != state)
    {
        auto wasProcessing(isProcessing());

        mPreviousState = mState;
        mState = state;

        if(isFinished())
        {
            mPriority += COMPLETED_PRIORITY_OFFSET;
        }
        else if(isPaused() && wasProcessing)
        {
            mPriority += ACTIVE_PRIORITY_OFFSET;
        }
        else if(isProcessing())
        {
            mPriority -= ACTIVE_PRIORITY_OFFSET;
        }
    }
}

void TransferData::setPreviousState(const TransferState &state)
{
    mPreviousState = state;
}

TransferData::TransferState TransferData::getState() const
{
    return mState;
}

TransferData::TransferState TransferData::getPreviousState() const
{
    return mPreviousState;
}

void TransferData::resetStateHasChanged()
{
    mPreviousState = mState;
}

bool TransferData::stateHasChanged() const
{
    return mState != mPreviousState;
}

int64_t TransferData::getRawFinishedTime() const
{
    return mFinishedTime;
}

int64_t TransferData::getSecondsSinceFinished() const
{
    auto preferences = Preferences::instance();
    return ((QDateTime::currentMSecsSinceEpoch()/ 100) - (preferences->getMsDiffTimeWithSDK() + mFinishedTime))/10;
}

QString TransferData::getFormattedFinishedTime() const
{
    auto preferences = Preferences::instance();
    qint64 secs = (preferences->getMsDiffTimeWithSDK() + mFinishedTime)/10;

    return QDateTime::fromTime_t(static_cast<uint>(secs)).toLocalTime().toString(QString::fromLatin1("hh:mm"));
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

bool TransferData::wasPaused() const
{
    return mPreviousState & TRANSFER_PAUSED;
}

bool TransferData::isProcessing() const
{
    return mState & PROCESSING_STATES_MASK;
}

bool TransferData::wasProcessing() const
{
    return mPreviousState & PROCESSING_STATES_MASK;
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

bool TransferData::wasFinished() const
{
    return mPreviousState & FINISHED_STATES_MASK;
}
