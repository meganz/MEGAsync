#include "TransferItem.h"
#include "megaapi.h"

using namespace mega;

TransferItem::TransferItem(QWidget *parent) : QWidget(parent)
{
    type = -1;
    transferError = 0;
    transferErrorValue = 0;
    totalSize = 0;
    totalTransferredBytes = 0;
    transferSpeed = 0;
    meanTransferSpeed = 0;
    regular = false;
    isLinkAvailable = false;
    nodeAccess = MegaShare::ACCESS_UNKNOWN;
    cancelButtonEnabled = false;
    isSyncTransfer = false;
    priority = 0;
    transferState = 0;
    transferTag = 0;
    dsFinishedTime = 0;
    transferFinishedWhileBlocked = false;
}

void TransferItem::setFileName(QString fileName)
{
    this->fileName = fileName;
}

QString TransferItem::getFileName()
{
    return fileName;
}

void TransferItem::setTransferredBytes(long long totalTransferredBytes, bool cancellable)
{
    this->totalTransferredBytes = totalTransferredBytes;
    if (this->totalTransferredBytes < 0)
    {
        this->totalTransferredBytes = 0;
    }
    if (this->totalTransferredBytes > this->totalSize)
    {
        this->totalTransferredBytes = this->totalSize;
    }
    regular = cancellable;
}

void TransferItem::setSpeed(long long transferSpeed, long long meanSpeed)
{
    if (transferSpeed < 0)
    {
        this->transferSpeed = 0;
    }
    else
    {
        this->transferSpeed = transferSpeed;
    }
    meanTransferSpeed = meanSpeed;
}

void TransferItem::setTotalSize(long long size)
{
    this->totalSize = size;
    if (this->totalSize < 0)
    {
        this->totalSize = 0;
    }
    if (this->totalTransferredBytes > this->totalSize)
    {
        this->totalTransferredBytes = this->totalSize;
    }
}

void TransferItem::setFinishedTime(long long time)
{
    dsFinishedTime = time;
}

long long TransferItem::getFinishedTime()
{
    return dsFinishedTime;
}

void TransferItem::setType(int type, bool isSyncTransfer)
{
    this->type = type;
    this->isSyncTransfer = isSyncTransfer;
}

int TransferItem::getType()
{
    return type;
}

void TransferItem::setPriority(unsigned long long priority)
{
    this->priority = priority;
}

unsigned long long TransferItem::getPriority()
{
    return this->priority;
}

int TransferItem::getTransferState()
{
    return transferState;
}

void TransferItem::setTransferState(int value)
{
    transferState = value;
}

bool TransferItem::isTransferFinished()
{
    return transferState == mega::MegaTransfer::STATE_COMPLETED
            || transferState == mega::MegaTransfer::STATE_FAILED;
}

int TransferItem::getTransferError()
{
    return transferError;
}

void TransferItem::setTransferError(int error, long long value)
{
    transferError = error;
    transferErrorValue = value;
}

int TransferItem::getTransferTag()
{
    return transferTag;    
}

void TransferItem::setTransferTag(int value)
{
    transferTag = value;    
}

bool TransferItem::getRegular()
{
    return regular;    
}

void TransferItem::setRegular(bool value)
{
    regular = value;    
}

bool TransferItem::getIsLinkAvailable()
{
    return isLinkAvailable;
}

void TransferItem::setIsLinkAvailable(bool value)
{
    isLinkAvailable = value;
}

int TransferItem::getNodeAccess()
{
    return nodeAccess;
}

void TransferItem::setNodeAccess(int value)
{
    nodeAccess = value;
}

bool TransferItem::getTransferFinishedWhileBlocked() const
{
    return transferFinishedWhileBlocked;
}

void TransferItem::setTransferFinishedWhileBlocked(bool value)
{
    transferFinishedWhileBlocked = value;
}
