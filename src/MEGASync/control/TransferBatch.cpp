#include "TransferBatch.h"

#include <QDir>

/*************************/
/*** TransferBatch *******/
/*************************/

TransferBatch::TransferBatch()
{
    mCancelToken = std::shared_ptr<mega::MegaCancelToken>(mega::MegaCancelToken::createInstance());
}

bool TransferBatch::isEmpty()
{
    return mPendingNodes.isEmpty();
}

void TransferBatch::add(const QString &nodePath)
{
    mPendingNodes.push_back(QDir::toNativeSeparators(nodePath));
}

void TransferBatch::cancel()
{
    mCancelToken->cancel();
}

void TransferBatch::onScanCompleted(const QString& nodePath)
{
    QString convertedNodePath = QDir::toNativeSeparators(nodePath);
    auto it = std::find(mPendingNodes.begin(), mPendingNodes.end(), convertedNodePath);
    if (it != mPendingNodes.end())
    {
        mPendingNodes.erase(it);
    }
}

QString TransferBatch::description()
{
    return QString::fromLatin1("%1 nodes").arg(mPendingNodes.size());
}

mega::MegaCancelToken* TransferBatch::getCancelTokenPtr()
{
    return mCancelToken.get();
}

std::shared_ptr<mega::MegaCancelToken> TransferBatch::getCancelToken()
{
    return mCancelToken;
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
        cancelled = true;
    }
}

void BlockingBatch::onScanCompleted(const QString& nodePath)
{
    if (mBatch)
    {
        mBatch->onScanCompleted(nodePath);
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

void BlockingBatch::onTransferFinished(const QString& nodePath)
{
    if (mBatch)
    {
        mBatch->onScanCompleted(nodePath);
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

bool BlockingBatch::isCancelled() const
{
    return cancelled;
}

bool BlockingBatch::hasNodes() const
{
    return (isValid()) ? !mBatch->isEmpty() : false;
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
   cancelled = false;
}
