#include "TransferItem2.h"
#include "megaapi.h"
#include "Utilities.h"

using namespace mega;

void TransferItem2::updateValuesTransferFinished(uint64_t updateTime,
                                  int errorCode, long long errorValue,
                                  long long meanSpeed,
                                  int state, long long transferedBytes)
{
   d->mErrorCode = errorCode;
   d->mState = state;
   d->mErrorValue = errorValue;
   d->mFinishedTime = updateTime;
   d->mRemainingTime = 0;
   d->mSpeed = 0;
   d->mMeanSpeed = meanSpeed;
   d->mTransferredBytes = transferedBytes;
   d->mUpdateTime = updateTime;
}

void TransferItem2::updateValuesTransferUpdated(uint64_t updateTime,
                                 int errorCode, long long errorValue,
                                 long long meanSpeed,
                                 long long speed,
                                 unsigned long long priority,
                                 int state, long long transferedBytes)
{
    d->mErrorCode = errorCode;
    d->mState = state;
    d->mErrorValue = errorValue;
    d->mRemainingTime = 50; // Use real value
    d->mSpeed = speed;
    d->mMeanSpeed = meanSpeed;
    d->mTransferredBytes = transferedBytes;
    d->mUpdateTime = updateTime;
    d->mPriority = priority;
}

void TransferItem2::setPaused(bool isPaused)
{
    if (isPaused)
    {
        d->mUnpausedState = d->mState;
        d->mState = MegaTransfer::STATE_PAUSED;
        d->mSpeed = 0;
    }
    else
    {
        d->mState = d->mUnpausedState;
    }
}
