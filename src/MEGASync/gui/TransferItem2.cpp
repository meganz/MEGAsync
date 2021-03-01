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
                                                uint64_t remainingTime,
                                 int errorCode, long long errorValue,
                                 long long meanSpeed,
                                 long long speed,
                                 unsigned long long priority,
                                 int state, long long transferedBytes)
{
    d->mState = state;
    d->mRemainingTime = remainingTime;
    d->mErrorCode = errorCode;
    d->mErrorValue = errorValue;
    d->mRemainingTime = remainingTime;
    d->mSpeed = speed;
    d->mMeanSpeed = meanSpeed;
    d->mTransferredBytes = transferedBytes;
    d->mUpdateTime = updateTime;
    d->mPriority = priority;
}
