#include "TransferItem.h"
#include "megaapi.h"

using namespace mega;

TransferItem::TransferItem(QWidget *parent) :
    QWidget(parent),
    mType(-1),
    mTransferError(0),
    mTransferErrorValue(0),
    mTotalSize(0),
    mTotalTransferredBytes(0),
    mTransferSpeed(0),
    mMeanTransferSpeed(0),
    mRegular(false),
    mIsLinkAvailable(false),
    mNodeAccess(MegaShare::ACCESS_UNKNOWN),
    mCancelButtonEnabled(false),
    mIsSyncTransfer(false),
    mPriority(0),
    mTransferState(0),
    mTransferTag(0),
    mDsFinishedTime(0),
    mTransferFinishedWhileBlocked(false)
{
}

void TransferItem::setFileName(QString fileName)
{
    mFileName = fileName;
}

QString TransferItem::getFileName()
{
    return mFileName;
}

void TransferItem::setTransferredBytes(long long totalTransferredBytes, bool cancellable)
{
    mTotalTransferredBytes = std::min(mTotalSize, std::max(0LL, totalTransferredBytes));
    mRegular = cancellable;
}

void TransferItem::setSpeed(long long transferSpeed, long long meanSpeed)
{
    mTransferSpeed = std::max(0LL, transferSpeed);
    mMeanTransferSpeed = meanSpeed;
}

void TransferItem::setTotalSize(long long size)
{ 
    mTotalSize = std::max(0LL, size);

    if (mTotalTransferredBytes > mTotalSize)
    {
        mTotalTransferredBytes = mTotalSize;
    }
}

void TransferItem::setFinishedTime(long long time)
{
    mDsFinishedTime = time;
}

long long TransferItem::getFinishedTime()
{
    return mDsFinishedTime;
}

void TransferItem::setType(int type, bool isSyncTransfer)
{
    mType = type;
    mIsSyncTransfer = isSyncTransfer;
}

int TransferItem::getType()
{
    return mType;
}

void TransferItem::setPriority(unsigned long long priority)
{
    mPriority = priority;
}

unsigned long long TransferItem::getPriority()
{
    return mPriority;
}

int TransferItem::getTransferState()
{
    return mTransferState;
}

void TransferItem::setTransferState(int value)
{
    mTransferState = value;
}

bool TransferItem::isTransferFinished()
{
    return mTransferState == mega::MegaTransfer::STATE_COMPLETED
            || mTransferState == mega::MegaTransfer::STATE_FAILED;
}

int TransferItem::getTransferError()
{
    return mTransferError;
}

void TransferItem::setTransferError(int error, long long value)
{
    mTransferError = error;
    mTransferErrorValue = value;
}

int TransferItem::getTransferTag()
{
    return mTransferTag;
}

void TransferItem::setTransferTag(int value)
{
    mTransferTag = value;
}

bool TransferItem::getRegular()
{
    return mRegular;
}

void TransferItem::setRegular(bool value)
{
    mRegular = value;
}

bool TransferItem::getIsLinkAvailable()
{
    return mIsLinkAvailable;
}

void TransferItem::setIsLinkAvailable(bool value)
{
    mIsLinkAvailable = value;
}

int TransferItem::getNodeAccess()
{
    return mNodeAccess;
}

void TransferItem::setNodeAccess(int value)
{
    mNodeAccess = value;
}

bool TransferItem::getTransferFinishedWhileBlocked() const
{
    return mTransferFinishedWhileBlocked;
}

void TransferItem::setTransferFinishedWhileBlocked(bool value)
{
    mTransferFinishedWhileBlocked = value;
}
