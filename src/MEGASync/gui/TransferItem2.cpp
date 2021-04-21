#include "TransferItem2.h"
#include "megaapi.h"
#include "Utilities.h"

using namespace mega;

void TransferItem2::updateValuesTransferFinished(int64_t finishTime,
                                                 int errorCode, long long errorValue,
                                                 long long meanSpeed,
                                                 TransferData::TransferState state,
                                                 long long transferedBytes,
                                                 MegaHandle parentHandle,
                                                 MegaHandle nodeHandle,
                                                 MegaNode* publicNode)
{
    d->mFinishedTime = finishTime;
    d->mErrorCode = errorCode;
    d->mState = state;
    d->mErrorValue = errorValue;
    d->mRemainingTime = 0LL;
    d->mSpeed = 0;
    d->mMeanSpeed = meanSpeed;
    d->mTransferredBytes = transferedBytes;
    d->mParentHandle = parentHandle;
    d->mNodeHandle = nodeHandle;
    d->mPublicNode = publicNode;
}

void TransferItem2::updateValuesTransferUpdated(int64_t remainingTime,
                                                int errorCode, long long errorValue,
                                                long long meanSpeed,
                                                long long speed,
                                                unsigned long long priority,
                                                TransferData::TransferState state,
                                                long long transferedBytes,
                                                MegaNode* publicNode)
{
    d->mState = state;
    d->mRemainingTime = remainingTime;
    d->mErrorCode = errorCode;
    d->mErrorValue = errorValue;
    d->mSpeed = speed;
    d->mMeanSpeed = meanSpeed;
    d->mTransferredBytes = transferedBytes;
    d->mPriority = priority;
    d->mPublicNode = publicNode;
}
