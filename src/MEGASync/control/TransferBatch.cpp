#include "TransferBatch.h"

/*************************/
/*** TransferBatch *******/
/*************************/

TransferBatch::TransferBatch()
{
    mCancelToken = std::shared_ptr<mega::MegaCancelToken>(mega::MegaCancelToken::createInstance());
}

bool TransferBatch::isEmpty()
{
    return (mFiles == 0 && mFolders == 0);
}

void TransferBatch::add(bool isDir)
{
    isDir ? ++mFolders : ++mFiles;
}

void TransferBatch::cancel()
{
    mCancelToken->cancel();
}

void TransferBatch::onFileScanCompleted()
{
    if (mFiles > 0)
    {
        --mFiles;
    }
}

void TransferBatch::onFolderScanCompleted()
{
    if (mFolders > 0)
    {
        --mFolders;
    }
}

void TransferBatch::onTransferFinished(bool isDir)
{
    isDir ? --mFolders : --mFiles;
}

QString TransferBatch::description()
{
    return QString::fromLatin1("%2 files, %3 folders").arg(mFiles).arg(mFolders);
}

mega::MegaCancelToken* TransferBatch::getCancelTokenPtr()
{
    return mCancelToken.get();
}

std::shared_ptr<mega::MegaCancelToken> TransferBatch::getCancelToken()
{
    return mCancelToken;
}

int TransferBatch::getFolderCount()
{
    return mFolders;
}

/*************************/
/*** BlockingBatch *******/
/*************************/

BlockingBatch::~BlockingBatch()
{
    clearBatch();
}

void BlockingBatch::add(std::shared_ptr<TransferBatch> _batch)
{
    mBatch = _batch;
}

void BlockingBatch::removeBatch()
{
    mBatch.reset();
}

void BlockingBatch::cancelTransfer()
{
    if (mBatch)
    {
        mBatch->cancel();
    }
}

void BlockingBatch::onFileScanCompleted()
{
    if (mBatch)
    {
        mBatch->onFileScanCompleted();
    }
}

void BlockingBatch::onFolderScanCompleted()
{
    if (mBatch)
    {
        mBatch->onFolderScanCompleted();
    }
}

bool BlockingBatch::isBlockingStageFinished()
{
    if (mBatch)
    {
        return mBatch->isEmpty();
    }
    return true;
}

void BlockingBatch::setAsUnblocked()
{
    clearBatch();
}

void BlockingBatch::onTransferFinished(bool isFolderTransfer)
{
    if (mBatch)
    {
        mBatch->onTransferFinished(isFolderTransfer);
        if (mBatch->isEmpty())
        {
            clearBatch();
        }
    }
}

bool BlockingBatch::hasCancelToken()
{
    return mBatch && mBatch->getCancelTokenPtr();
}

bool BlockingBatch::isValid() const
{
    return mBatch != nullptr;
}

bool BlockingBatch::hasFolders() const
{
    return (isValid()) ? mBatch->getFolderCount() > 0 : false;
}

std::shared_ptr<mega::MegaCancelToken> BlockingBatch::getCancelToken()
{
    if (mBatch)
    {
        return mBatch->getCancelToken();
    }
    return std::shared_ptr<mega::MegaCancelToken>();
}

QString BlockingBatch::description()
{
    if (mBatch)
    {
        return mBatch->description();
    }
    return QString::fromLatin1("nullptr");
}

void BlockingBatch::clearBatch()
{
   mBatch = nullptr;
}
