#include "TransferBatch.h"

#include <MegaApplication.h>

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

void TransferBatch::add(const QString &nodePath, const QString& nodeName)
{
    auto nodePathWithNativeSeparators = QDir::toNativeSeparators(nodePath);
    if(!nodeName.isEmpty())
    {
        if(!nodePathWithNativeSeparators.endsWith(QDir::separator()))
        {
            nodePathWithNativeSeparators = nodePathWithNativeSeparators + QDir::separator();
        }

        auto escapedChar = MegaSyncApp->getMegaApi()->unescapeFsIncompatible(nodeName.toStdString().c_str());
        nodePathWithNativeSeparators = nodePathWithNativeSeparators + QString::fromUtf8(escapedChar);
        delete [] escapedChar;
    }

    mPendingNodes.push_back(nodePathWithNativeSeparators);
}

void TransferBatch::cancel()
{
    mCancelToken->cancel();
}

void TransferBatch::onScanCompleted(const QString& nodePath)
{
    std::string nodePathCopy = nodePath.toStdString();
    auto escapedChar = MegaSyncApp->getMegaApi()->unescapeFsIncompatible(nodePathCopy.c_str());
    QString convertedNodePath = QDir::toNativeSeparators(QString::fromUtf8(escapedChar));
    delete [] escapedChar;

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
    if (isValid())
    {
        mBatch->cancel();
        cancelled = true;
    }
}

void BlockingBatch::onScanCompleted(const QString& nodePath)
{
    if (isValid())
    {
        mBatch->onScanCompleted(nodePath);
    }
}

bool BlockingBatch::isBlockingStageFinished()
{
    if (isValid())
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
    if (isValid())
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
    if (isValid())
    {
        return mBatch->getCancelToken();
    }
    return std::shared_ptr<mega::MegaCancelToken>();
}

QString BlockingBatch::description()
{
    if (isValid())
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
