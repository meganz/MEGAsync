#include "TransferItem2.h"
#include "megaapi.h"
#include "Utilities.h"

using namespace mega;

TransferItem2::TransferItem2() : d(new TransferDataRow()) {}
TransferItem2::TransferItem2(const TransferItem2& ti)  : d(new TransferDataRow(ti.d.constData())) {}
TransferItem2::TransferItem2(const TransferDataRow& dataRow) : d(new TransferDataRow(dataRow)) {}

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

QSharedDataPointer<TransferDataRow> TransferItem2::getTransferData() const
{
    return d;
}
